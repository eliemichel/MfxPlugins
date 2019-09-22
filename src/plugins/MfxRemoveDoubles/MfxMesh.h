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

#pragma once

#include "ofxCore.h"
#include "ofxMeshEffect.h"

class MfxMesh {
public:
	int point_count;
	int vertex_count;
	int face_count;
	float *point_data;
	int *vertex_data;
	int *face_data;

public:
	MfxMesh(OfxMeshInputHandle input,
		    OfxTime time,
		    OfxPropertySuiteV1 *propertySuite,
		    OfxMeshEffectSuiteV1 *meshEffectSuite);
	~MfxMesh();

	/**
	 * Get mesh data (point/vert/face counts and data pointers) from a mesh handle
	 */
	void getData();

	/**
	 * Allocate mesh data to match the value of point/vertex/face_count members
	 * and get new pointers
	 */
	void allocateData();

private:
	OfxPropertySetHandle m_mesh_handle;
	OfxPropertySuiteV1 *m_propertySuite;
	OfxMeshEffectSuiteV1 *m_meshEffectSuite;
};
