/*
 *
 * Copyright (c) 2015, alipay.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * 	dalvik_method_replace.cpp
 *
 * @author : sanping.li@alipay.com
 *
 */
#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

#include <stdbool.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "dalvik.h"
#include "common.h"

static void* dvm_dlsym(void *hand, const char *name) {
	void* ret = dlsym(hand, name);
	char msg[1024] = { 0 };
	snprintf(msg, sizeof(msg) - 1, "0x%x", ret);
	LOGD("%s = %s\n", name, msg);
	return ret;
}

extern jboolean __attribute__ ((visibility ("hidden"))) dalvik_setup(
		JNIEnv* env, int apilevel) {
	void* dvm_hand = dlopen("libdvm.so", RTLD_NOW);
	if (dvm_hand) {
		dvmDecodeIndirectRef_fnPtr = dvm_dlsym(dvm_hand,
				apilevel > 10 ?
						"_Z20dvmDecodeIndirectRefP6ThreadP8_jobject" :
						"dvmDecodeIndirectRef");
		if (!dvmDecodeIndirectRef_fnPtr) {
			return JNI_FALSE;
		}
		dvmThreadSelf_fnPtr = dvm_dlsym(dvm_hand,
				apilevel > 10 ? "_Z13dvmThreadSelfv" : "dvmThreadSelf");
		if (!dvmThreadSelf_fnPtr) {
			return JNI_FALSE;
		}
		jclass clazz = env->FindClass("java/lang/reflect/Method");
		jClassMethod = env->GetMethodID(clazz, "getDeclaringClass",
						"()Ljava/lang/Class;");

		return JNI_TRUE;
	} else {
		return JNI_FALSE;
	}
}

extern void __attribute__ ((visibility ("hidden"))) dalvik_replaceMethod(
		JNIEnv* env, jobject src, jobject dest) {
	jobject clazz = env->CallObjectMethod(dest, jClassMethod);
	ClassObject* clz = (ClassObject*) dvmDecodeIndirectRef_fnPtr(
			dvmThreadSelf_fnPtr(), clazz);
	clz->status = CLASS_INITIALIZED;

	Method* meth = (Method*) env->FromReflectedMethod(src);
	Method* target = (Method*) env->FromReflectedMethod(dest);
	LOGD("dalvikMethod: %s", meth->name);

	meth->clazz = target->clazz;
	meth->accessFlags |= ACC_PUBLIC;
	meth->methodIndex = target->methodIndex;
	meth->jniArgInfo = target->jniArgInfo;
	meth->registersSize = target->registersSize;
	meth->outsSize = target->outsSize;
	meth->insSize = target->insSize;

	meth->prototype = target->prototype;
	meth->insns = target->insns;
	meth->nativeFunc = target->nativeFunc;
}

extern void dalvik_setFieldFlag(JNIEnv* env, jobject field) {
	Field* dalvikField = (Field*) env->FromReflectedField(field);
	dalvikField->accessFlags = dalvikField->accessFlags & (~ACC_PRIVATE)
			| ACC_PUBLIC;
	LOGD("dalvik_setFieldFlag: %d ", dalvikField->accessFlags);
}
