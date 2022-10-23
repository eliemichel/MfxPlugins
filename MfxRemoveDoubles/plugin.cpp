/**
 * This file is part of MfxPlugins
 *
 * Copyright (c) 2019-2022 -- Élie Michel <elie.michel@exppad.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * The Software is provided “as is”, without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and non-infringement. In no event shall the
 * authors or copyright holders be liable for any claim, damages or other
 * liability, whether in an action of contract, tort or otherwise, arising
 * from, out of or in connection with the software or the use or other dealings
 * in the Software.
 */

#include "KDTree.h"

#include <OpenMfx/Sdk/Cpp/Plugin/MfxEffect>
#include <OpenMfx/Sdk/Cpp/Plugin/MfxRegister>

#include <iostream>
#include <cassert>
#include <unordered_set>

///////////////////////////////////////////////////////////////////////////////
// Remove Doubles

class RemoveDoublesEffect : public MfxEffect {
public:
	const char* GetName() override
	{ return "RemoveDoubles"; }

protected:
	OfxStatus Describe(OfxMeshEffectHandle descriptor) override {
		AddInput(kOfxMeshMainInput);
		AddInput(kOfxMeshMainOutput);

		AddParam("threshold", 0.0001)
			.Label("Threshold");

		return kOfxStatOK;
	}

	OfxStatus Cook(OfxMeshEffectHandle instance) override {
		// 0. Get input data
		MfxMesh inputMesh = GetInput(kOfxMeshMainInput).GetMesh();
		MfxMesh outputMesh = GetInput(kOfxMeshMainOutput).GetMesh();
		double threshold = GetParam<double>("threshold").GetValue();

		MfxMeshProps inputMeshProps;
		inputMesh.FetchProperties(inputMeshProps);

		MfxAttribute inputPos = inputMesh.GetPointAttribute(kOfxMeshAttribPointPosition);
		MfxAttributeProps inputPosProps;
		inputPos.FetchProperties(inputPosProps);

		MfxAttribute inputCorner = inputMesh.GetCornerAttribute(kOfxMeshAttribCornerPoint);
		MfxAttributeProps inputCornerProps;
		inputCorner.FetchProperties(inputCornerProps);

		MfxAttribute inputFaceSize = inputMesh.GetFaceAttribute(kOfxMeshAttribFaceSize);
		MfxAttributeProps inputFaceSizeProps;
		inputFaceSize.FetchProperties(inputFaceSizeProps);

		// 1. Build kd-tree
		KDTree tree(inputMeshProps.pointCount, inputPosProps.data, inputPosProps.stride);

		std::vector<int> assign(inputMeshProps.pointCount);
		std::vector<int> offset(inputMeshProps.pointCount);
		int removed_points = 0;
		for (int i = 0; i < inputMeshProps.pointCount; ++i) {
			int equiv = tree.equivalent(i, static_cast<float>(threshold));
			assert(equiv <= i);
			assert(equiv >= 0);
			if (equiv != i) {
				// TODO: this fixes a bug in the kdtree implementation to ensure the equivalent point is not removed
				// loop terminates eventually because 0 <= equiv_n+1 < equiv_n
				while (assign[equiv] != equiv) {
					equiv = assign[equiv];
				}
				assert(assign[equiv] == equiv); // equivalent point is not removed
				++removed_points;
			}
			assign[i] = equiv;
			offset[i] = removed_points;
			assert(i - offset[i] < i - removed_points + 1);
		}
		std::cout << "Removing " << removed_points << " points" << std::endl;

		int removed_corners = 0;
		std::vector<int> vertex_offset(inputMeshProps.cornerCount);
#ifndef NDEBUG
		// store to check integrity
		std::vector<int> keep_face(inputMeshProps.cornerCount);
#endif // NDEBUG
		std::vector<int> new_face_data;
		new_face_data.reserve(inputMeshProps.faceCount);
		int v = 0;
		for (int i = 0; i < inputMeshProps.faceCount; ++i) {
			// If several vertices refer to points which are now merged (same 'new_p'), then we remove extras
			std::unordered_set<int> new_points;
			int old_f = *inputFaceSizeProps.at<int>(i);
			int new_f = old_f;
			for (int j = 0; j < old_f; ++j) {
				int new_p = assign[*inputCornerProps.at<int>(v + j)];
				vertex_offset[v + j] = removed_corners;
				if (new_points.count(new_p) > 0) {
					++removed_corners;
					--new_f;
				}
				else {
					new_points.insert(new_p);
				}
			}
			if (new_f > 1) {
				new_face_data.push_back(new_f);
#ifndef NDEBUG
				keep_face[i] = 1;
#endif // NDEBUG
			}
			else {
				// Rewrite this face's vertices offsets to remove all vertices
				removed_corners = vertex_offset[v];
				for (int j = 1; j < old_f; ++j) {
					++removed_corners;
					vertex_offset[v + j] = removed_corners;
				}
				++removed_corners;
#ifndef NDEBUG
				keep_face[i] = 0;
#endif // NDEBUG
			}
			v += old_f;
		}

		std::cout << "Removing " << removed_corners << " vertices" << std::endl;

		// 2. Get new point count and allocate data
		int outputPointCount = inputMeshProps.pointCount - removed_points;
		int outputCornerCount = inputMeshProps.cornerCount - removed_corners;
		int outputFaceCount = static_cast<int>(new_face_data.size());
		outputMesh.Allocate(outputPointCount, outputCornerCount, outputFaceCount);

		MfxAttribute outputPos = outputMesh.GetPointAttribute(kOfxMeshAttribPointPosition);
		MfxAttributeProps outputPosProps;
		outputPos.FetchProperties(outputPosProps);

		MfxAttribute outputCorner = outputMesh.GetCornerAttribute(kOfxMeshAttribCornerPoint);
		MfxAttributeProps outputCornerProps;
		outputCorner.FetchProperties(outputCornerProps);

		MfxAttribute outputFaceSize = outputMesh.GetFaceAttribute(kOfxMeshAttribFaceSize);
		MfxAttributeProps outputFaceSizeProps;
		outputFaceSize.FetchProperties(outputFaceSizeProps);

		// 3. Fill output
		for (int i = 0; i < inputMeshProps.pointCount; ++i) {
			if (assign[i] == i) {
				int j = i - offset[i];
				assert(j < outputPointCount);
				const float* inP = inputPosProps.at<float>(j);
				float* outP = outputPosProps.at<float>(j);
				memcpy(outP, inP, 3 * sizeof(float));
			}
		}
		for (int i = 0; i < inputMeshProps.cornerCount; ++i) {
			bool is_removed = vertex_offset[i] != (i < inputMeshProps.cornerCount - 1 ? vertex_offset[i + 1] : removed_corners);
			if (!is_removed) {
				int p = assign[*inputCornerProps.at<int>(i)];
				assert(p - offset[p] < outputPointCount);
				assert(i - vertex_offset[i] < outputCornerCount);
				*outputCornerProps.at<int>(i - vertex_offset[i]) = p - offset[p];
			}
		}
		// Check integrity
#ifndef NDEBUG
		v = 0;
		for (int i = 0; i < inputMeshProps.faceCount; ++i) {
			int f = *inputFaceSizeProps.at<int>(i);
			int count_removed = 0;
			for (int j = 0; j < f; ++j) {
				bool is_removed = vertex_offset[v + j] != (v + j < inputMeshProps.cornerCount - 1 ? vertex_offset[v + j + 1] : removed_corners);
				count_removed += is_removed;
			}
			assert(count_removed == f || (keep_face[i] && (f - count_removed > 1)));
			v += f;
		}
#endif // NDEBUG
		if (outputFaceSizeProps.stride != sizeof(int)) {
			return kOfxStatErrUnsupported;
		}
		memcpy(outputFaceSizeProps.data, new_face_data.data(), outputFaceCount * sizeof(int));

		inputMesh.Release();
		outputMesh.Release();
		return kOfxStatOK;
	}
};

///////////////////////////////////////////////////////////////////////////////

MfxRegister(
	RemoveDoublesEffect
);
