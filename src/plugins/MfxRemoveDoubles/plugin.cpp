/**
 * This file is part of MfxPlugins
 *
 * Copyright (c) 2019 -- Élie Michel <elie.michel@exppad.com>
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

#include <iostream>
#include <cassert>
#include <unordered_set>
#include "MfxPlugin.h"
#include "KDTree.h"

///////////////////////////////////////////////////////////////////////////////
// Remove Doubles

class RemoveDoublesPlugin : public MfxPlugin {
public:
	RemoveDoublesPlugin() {
		ofxPlugin.pluginIdentifier = "RemoveDoubles";
	}

	void describeParameters(OfxParamSetHandle parameters) override {
		parameterSuite->paramDefine(parameters, kOfxParamTypeDouble, "Threshold", NULL);
	}
	
	bool cookCore(MfxMesh & input_mesh,
			      MfxMesh & output_mesh,
			      OfxParamSetHandle parameters) override {
		// 0. Get parameters
		double threshold;
		OfxParamHandle param;
		parameterSuite->paramGetHandle(parameters, "Threshold", &param, NULL);
		parameterSuite->paramGetValue(param, &threshold);

		// 1. Build kd-tree
		input_mesh.getData();
		KDTree tree(input_mesh.point_count, input_mesh.point_data);

		std::vector<int> assign(input_mesh.point_count);
		std::vector<int> offset(input_mesh.point_count);
		int removed_points = 0;
		for (int i = 0 ; i < input_mesh.point_count ; ++i) {
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

		int removed_vertices = 0;
		std::vector<int> vertex_offset(input_mesh.vertex_count);
#ifndef NDEBUG
		// store to check integrity
		std::vector<int> keep_face(input_mesh.vertex_count);
#endif // NDEBUG
		std::vector<int> new_face_data;
		new_face_data.reserve(input_mesh.face_count);
		int v = 0;
		for (int i = 0 ; i < input_mesh.face_count ; ++i) {
			// If several vertices refer to points which are now merged (same 'new_p'), then we remove extras
			std::unordered_set<int> new_points;
			int old_f = input_mesh.face_data[i];
			int new_f = old_f;
			for (int j = 0 ; j < old_f ; ++j) {
				int new_p = assign[input_mesh.vertex_data[v + j]];
				vertex_offset[v + j] = removed_vertices;
				if (new_points.count(new_p) > 0) {
					++removed_vertices;
					--new_f;
				} else {
					new_points.insert(new_p);
				}
			}
			if (new_f > 1) {
				new_face_data.push_back(new_f);
#ifndef NDEBUG
				keep_face[i] = 1;
#endif // NDEBUG
			} else {
				// Rewrite this face's vertices offsets to remove all vertices
				removed_vertices = vertex_offset[v];
				for (int j = 1 ; j < old_f ; ++j) {
					++removed_vertices;
					vertex_offset[v + j] = removed_vertices;
				}
				++removed_vertices;
#ifndef NDEBUG
				keep_face[i] = 0;
#endif // NDEBUG
			}
			v += old_f;
		}

		std::cout << "Removing " << removed_vertices << " vertices" << std::endl;

		// 2. Get new point count and allocate data
		output_mesh.point_count = input_mesh.point_count - removed_points;
		output_mesh.vertex_count = input_mesh.vertex_count - removed_vertices;
		output_mesh.face_count = static_cast<int>(new_face_data.size());
    	output_mesh.allocateData();

		// 3. Fill output
		for (int i = 0 ; i < input_mesh.point_count ; ++i) {
			if (assign[i] == i) {
				int j = i - offset[i];
				assert(j < output_mesh.point_count);
				output_mesh.point_data[3 * j + 0] = input_mesh.point_data[3 * i + 0];
				output_mesh.point_data[3 * j + 1] = input_mesh.point_data[3 * i + 1];
				output_mesh.point_data[3 * j + 2] = input_mesh.point_data[3 * i + 2];
			}
		}
		for (int i = 0 ; i < input_mesh.vertex_count ; ++i) {
			bool is_removed = vertex_offset[i] != (i < input_mesh.vertex_count - 1 ? vertex_offset[i + 1] : removed_vertices);
			if (!is_removed) {
				int p = assign[input_mesh.vertex_data[i]];
				assert(p - offset[p] < output_mesh.point_count);
				assert(i - vertex_offset[i] < output_mesh.vertex_count);
				output_mesh.vertex_data[i - vertex_offset[i]] = p - offset[p];
			}
		}
		// Check integrity
#ifndef NDEBUG
		v = 0;
		for (int i = 0 ; i < input_mesh.face_count ; ++i) {
			int f = input_mesh.face_data[i];
			int count_removed = 0;
			for (int j = 0 ; j < f ; ++j) {
				bool is_removed = vertex_offset[v + j] != (v + j < input_mesh.vertex_count - 1 ? vertex_offset[v + j + 1] : removed_vertices);
				count_removed += is_removed;
			}
			assert(count_removed == f || (keep_face[i] && (f - count_removed > 1)));
			v += f;
		}
#endif // NDEBUG
		memcpy(output_mesh.face_data, new_face_data.data(), output_mesh.face_count * sizeof(int));

    	return true;
	}

};

///////////////////////////////////////////////////////////////////////////////
// Entry points

RemoveDoublesPlugin plugin;

static void plugin_setHost(OfxHost *host) {
	plugin.setHost(host);
}
static OfxStatus plugin_mainEntry(const char *action,
	                                         const void *handle,
	                                         OfxPropertySetHandle inArgs,
	                                         OfxPropertySetHandle outArgs) {
	return plugin.mainEntry(action, handle, inArgs, outArgs);
}

OfxExport int OfxGetNumberOfPlugins(void) {
	plugin.ofxPlugin.mainEntry = plugin_mainEntry;
	plugin.ofxPlugin.setHost = plugin_setHost;
    return 1;
}

OfxExport OfxPlugin *OfxGetPlugin(int nth) {
	return &plugin.ofxPlugin;
}
