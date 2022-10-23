/**
 * This file is part of MfxPlugins
 *
 * Copyright (c) 2019 - 2020 -- Élie Michel <elie.michel@exppad.com>
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

/**
 * Translate modifier is a very simple demo showcasing the C++ API
 */

#include <OpenMfx/Sdk/Cpp/Plugin/MfxEffect>
#include <OpenMfx/Sdk/Cpp/Plugin/MfxRegister>

///////////////////////////////////////////////////////////////////////////////

void copyAndTranslate(MfxAttribute& outputPosAttr, MfxAttribute& inputPosAttr, int pointCount, const double3& translation)
{
	MfxAttributeProps inputPos, outputPos;
	inputPosAttr.FetchProperties(inputPos);
	outputPosAttr.FetchProperties(outputPos);

	const char* src = inputPos.data;
	char* dst = outputPos.data;
	for (int i = 0; i < pointCount; ++i) {
		for (int k = 0; k < 3; ++k) {
			((float*)dst)[k] = ((float*)src)[k] + (float)translation[k];
		}
		src += inputPos.stride;
		dst += outputPos.stride;
	}
}

class TranslateEffect : public MfxEffect {
public:
	const char* GetName() override
	{ return "Translate"; }

protected:
	OfxStatus Describe(OfxMeshEffectHandle descriptor) override {
		AddInput(kOfxMeshMainInput);
		AddInput(kOfxMeshMainOutput);

		AddParam("translation", double3{ 0.0, 0.0, 0.0 })
			.Label("Translation");

		return kOfxStatOK;
	}

	OfxStatus Cook(OfxMeshEffectHandle instance) override {
		MfxMesh inputMesh = GetInput(kOfxMeshMainInput).GetMesh();
		MfxMesh outputMesh = GetInput(kOfxMeshMainOutput).GetMesh();
		double3 translation = GetParam<double3>("translation").GetValue();

		MfxMeshProps inputMeshProps;
		inputMesh.FetchProperties(inputMeshProps);

		outputMesh.Allocate(inputMeshProps.pointCount, inputMeshProps.cornerCount, inputMeshProps.faceCount);

		MfxAttribute inputPos = inputMesh.GetPointAttribute(kOfxMeshAttribPointPosition);
		MfxAttribute outputPos = outputMesh.GetPointAttribute(kOfxMeshAttribPointPosition);
		copyAndTranslate(outputPos, inputPos, inputMeshProps.pointCount, translation);

		MfxAttribute inputPoints = inputMesh.GetCornerAttribute(kOfxMeshAttribCornerPoint);
		MfxAttribute outputPoints = outputMesh.GetCornerAttribute(kOfxMeshAttribCornerPoint);
		outputPoints.CopyFrom(inputPoints, 0, inputMeshProps.cornerCount);

		MfxAttribute inputFaces = inputMesh.GetFaceAttribute(kOfxMeshAttribFaceSize);
		MfxAttribute outputFaces = outputMesh.GetFaceAttribute(kOfxMeshAttribFaceSize);
		outputFaces.CopyFrom(inputFaces, 0, inputMeshProps.faceCount);

		inputMesh.Release();
		outputMesh.Release();
		return kOfxStatOK;
	}
};

///////////////////////////////////////////////////////////////////////////////

MfxRegister(
	TranslateEffect
);
