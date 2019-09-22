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

#include <cstring>

#include "MfxPlugin.h"
#include "MfxMesh.h"

MfxPlugin::MfxPlugin() {
	ofxPlugin.pluginApi = kOfxMeshEffectPluginApi;
	ofxPlugin.apiVersion = kOfxMeshEffectPluginApiVersion;
	ofxPlugin.pluginVersionMajor = 1;
	ofxPlugin.pluginVersionMinor = 0;
}

OfxStatus MfxPlugin::load() {
    return kOfxStatOK;
}

OfxStatus MfxPlugin::unload() {
    return kOfxStatOK;
}

OfxStatus MfxPlugin::describe(OfxMeshEffectHandle descriptor) {
    bool missing_suite =
        NULL == propertySuite ||
        NULL == parameterSuite ||
        NULL == meshEffectSuite;
    if (missing_suite) {
        return kOfxStatErrMissingHostFeature;
    }

    OfxPropertySetHandle inputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainInput, &inputProperties);
    propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input");

    OfxPropertySetHandle outputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainOutput, &outputProperties);
    propertySuite->propSetString(outputProperties, kOfxPropLabel, 0, "Main Output");

    OfxParamSetHandle parameters;
    meshEffectSuite->getParamSet(descriptor, &parameters);

	describeParameters(parameters);

    return kOfxStatOK;
}

OfxStatus MfxPlugin::createInstance(OfxMeshEffectHandle instance) {
    return kOfxStatOK;
}

OfxStatus MfxPlugin::destroyInstance(OfxMeshEffectHandle instance) {
    return kOfxStatOK;
}

OfxStatus MfxPlugin::cook(OfxMeshEffectHandle instance) {
    OfxTime time = 0;

    // Get input/output
    OfxMeshInputHandle input, output;
    meshEffectSuite->inputGetHandle(instance, kOfxMeshMainInput, &input, NULL);
    meshEffectSuite->inputGetHandle(instance, kOfxMeshMainOutput, &output, NULL);

    // Get meshes
    MfxMesh input_mesh(input, time, propertySuite, meshEffectSuite);
    MfxMesh output_mesh(output, time, propertySuite, meshEffectSuite);

	// Get parameters
	OfxParamSetHandle parameters;
	meshEffectSuite->getParamSet(instance, &parameters);

	return
		cookCore(input_mesh, output_mesh, parameters)
		? kOfxStatOK
		: kOfxStatFailed;
}

OfxStatus MfxPlugin::mainEntry(const char *action,
	                const void *handle,
	                OfxPropertySetHandle inArgs,
	                OfxPropertySetHandle outArgs) {
	if (0 == strcmp(action, kOfxActionLoad)) {
		return load();
	}
	if (0 == strcmp(action, kOfxActionUnload)) {
		return unload();
	}
	if (0 == strcmp(action, kOfxActionDescribe)) {
		return describe((OfxMeshEffectHandle)handle);
	}
	if (0 == strcmp(action, kOfxActionCreateInstance)) {
		return createInstance((OfxMeshEffectHandle)handle);
	}
	if (0 == strcmp(action, kOfxActionDestroyInstance)) {
		return destroyInstance((OfxMeshEffectHandle)handle);
	}
	if (0 == strcmp(action, kOfxMeshEffectActionCook)) {
		return cook((OfxMeshEffectHandle)handle);
	}
	return kOfxStatReplyDefault;
}

void MfxPlugin::setHost(OfxHost *_host) {
	host = _host;
	if (NULL != host) {
		propertySuite = (OfxPropertySuiteV1*)host->fetchSuite(host->host, kOfxPropertySuite, 1);
		parameterSuite = (OfxParameterSuiteV1*)host->fetchSuite(host->host, kOfxParameterSuite, 1);
		meshEffectSuite = (OfxMeshEffectSuiteV1*)host->fetchSuite(host->host, kOfxMeshEffectSuite, 1);
	}
}

