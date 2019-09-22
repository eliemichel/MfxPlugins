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

#include "MfxMesh.h"

class MfxPlugin {
public:
	MfxPlugin();

	OfxStatus load();
	OfxStatus unload();
	OfxStatus describe(OfxMeshEffectHandle descriptor);
	OfxStatus createInstance(OfxMeshEffectHandle instance);
	OfxStatus destroyInstance(OfxMeshEffectHandle instance);
	OfxStatus cook(OfxMeshEffectHandle instance);
	
	OfxStatus mainEntry(const char *action,
	                    const void *handle,
	                    OfxPropertySetHandle inArgs,
	                    OfxPropertySetHandle outArgs);

	void setHost(OfxHost *_host);

	virtual void describeParameters(OfxParamSetHandle parameters) = 0;
	virtual bool cookCore(MfxMesh & input_mesh,
	                      MfxMesh & output_mesh,
	                      OfxParamSetHandle parameters) = 0;

public:
	OfxPlugin ofxPlugin;
	OfxHost *host;
    OfxPropertySuiteV1 *propertySuite;
    OfxParameterSuiteV1 *parameterSuite;
    OfxMeshEffectSuiteV1 *meshEffectSuite;
};
