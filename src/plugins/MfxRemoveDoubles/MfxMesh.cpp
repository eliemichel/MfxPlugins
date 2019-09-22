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

#include "MfxMesh.h"

MfxMesh::MfxMesh(OfxMeshInputHandle input, OfxTime time, OfxPropertySuiteV1 *propertySuite, OfxMeshEffectSuiteV1 *meshEffectSuite)
	: point_count(0)
	, vertex_count(0)
	, face_count(0)
	, point_data(nullptr)
	, vertex_data(nullptr)
	, face_data(nullptr)
	, m_propertySuite(propertySuite)
	, m_meshEffectSuite(meshEffectSuite)
{
	m_meshEffectSuite->inputGetMesh(input, time, &m_mesh_handle);
}

MfxMesh::~MfxMesh() {
	m_meshEffectSuite->inputReleaseMesh(m_mesh_handle);
}

void MfxMesh::getData() {
	m_propertySuite->propGetInt(m_mesh_handle, kOfxMeshPropPointCount,
                                0, &point_count);
    m_propertySuite->propGetInt(m_mesh_handle, kOfxMeshPropVertexCount,
                                0, &vertex_count);
    m_propertySuite->propGetInt(m_mesh_handle, kOfxMeshPropFaceCount,
                                0, &face_count);
    m_propertySuite->propGetPointer(m_mesh_handle, kOfxMeshPropPointData,
                                    0, (void**)&point_data);
    m_propertySuite->propGetPointer(m_mesh_handle, kOfxMeshPropVertexData,
                                    0, (void**)&vertex_data);
    m_propertySuite->propGetPointer(m_mesh_handle, kOfxMeshPropFaceData,
                                    0, (void**)&face_data);
}

void MfxMesh::allocateData() {
	m_meshEffectSuite->meshAlloc(m_mesh_handle, point_count, vertex_count, face_count);
	getData();
}
