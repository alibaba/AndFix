/*
 *
 * Copyright (c) 2008 The Android Open Source Project
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

/*
 * dalvik.h
 *
 *
 * @author : sanping.li@alipay.com
 *
 */
#include <string.h>
#include <jni.h>
#include <stdio.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdint.h>    /* C99 */

#include "common.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;
typedef int8_t s1;
typedef int16_t s2;
typedef int32_t s4;
typedef int64_t s8;

/*
 * access flags and masks; the "standard" ones are all <= 0x4000
 *
 * Note: There are related declarations in vm/oo/Object.h in the ClassFlags
 * enum.
 */
enum {
	ACC_PUBLIC = 0x00000001,       // class, field, method, ic
	ACC_PRIVATE = 0x00000002,       // field, method, ic
	ACC_PROTECTED = 0x00000004,       // field, method, ic
	ACC_STATIC = 0x00000008,       // field, method, ic
	ACC_FINAL = 0x00000010,       // class, field, method, ic
	ACC_SYNCHRONIZED = 0x00000020,       // method (only allowed on natives)
	ACC_SUPER = 0x00000020,       // class (not used in Dalvik)
	ACC_VOLATILE = 0x00000040,       // field
	ACC_BRIDGE = 0x00000040,       // method (1.5)
	ACC_TRANSIENT = 0x00000080,       // field
	ACC_VARARGS = 0x00000080,       // method (1.5)
	ACC_NATIVE = 0x00000100,       // method
	ACC_INTERFACE = 0x00000200,       // class, ic
	ACC_ABSTRACT = 0x00000400,       // class, method, ic
	ACC_STRICT = 0x00000800,       // method
	ACC_SYNTHETIC = 0x00001000,       // field, method, ic
	ACC_ANNOTATION = 0x00002000,       // class, ic (1.5)
	ACC_ENUM = 0x00004000,       // class, field, ic (1.5)
	ACC_CONSTRUCTOR = 0x00010000,       // method (Dalvik only)
	ACC_DECLARED_SYNCHRONIZED = 0x00020000,       // method (Dalvik only)
	ACC_CLASS_MASK = (ACC_PUBLIC | ACC_FINAL | ACC_INTERFACE | ACC_ABSTRACT
			| ACC_SYNTHETIC | ACC_ANNOTATION | ACC_ENUM),
	ACC_INNER_CLASS_MASK = (ACC_CLASS_MASK | ACC_PRIVATE | ACC_PROTECTED
			| ACC_STATIC),
	ACC_FIELD_MASK = (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC
			| ACC_FINAL | ACC_VOLATILE | ACC_TRANSIENT | ACC_SYNTHETIC
			| ACC_ENUM),
	ACC_METHOD_MASK = (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC
			| ACC_FINAL | ACC_SYNCHRONIZED | ACC_BRIDGE | ACC_VARARGS
			| ACC_NATIVE | ACC_ABSTRACT | ACC_STRICT | ACC_SYNTHETIC
			| ACC_CONSTRUCTOR | ACC_DECLARED_SYNCHRONIZED),
};

typedef struct DexProto {
	u4* dexFile; /* file the idx refers to */
	u4 protoIdx; /* index into proto_ids table of dexFile */
} DexProto;

typedef void (*DalvikBridgeFunc)(const u4* args, void* pResult,
		const void* method, void* self);

struct Field {
	void* clazz; /* class in which the field is declared */
	const char* name;
	const char* signature; /* e.g. "I", "[C", "Landroid/os/Debug;" */
	u4 accessFlags;
};

struct Method;
struct ClassObject;
struct ArrayObject;

typedef struct Object {
	/* ptr to class object */
	struct ClassObject* clazz;

	/*
	 * A word containing either a "thin" lock or a "fat" monitor.  See
	 * the comments in Sync.c for a description of its layout.
	 */
	u4 lock;
} Object;

struct InitiatingLoaderList {
	/* a list of initiating loader Objects; grown and initialized on demand */
	void** initiatingLoaders;
	/* count of loaders in the above list */
	int initiatingLoaderCount;
};

enum PrimitiveType {
	PRIM_NOT = 0, /* value is a reference type, not a primitive type */
	PRIM_VOID = 1,
	PRIM_BOOLEAN = 2,
	PRIM_BYTE = 3,
	PRIM_SHORT = 4,
	PRIM_CHAR = 5,
	PRIM_INT = 6,
	PRIM_LONG = 7,
	PRIM_FLOAT = 8,
	PRIM_DOUBLE = 9,
}typedef PrimitiveType;

enum ClassStatus {
	CLASS_ERROR = -1,

	CLASS_NOTREADY = 0, CLASS_IDX = 1, /* loaded, DEX idx in super or ifaces */
	CLASS_LOADED = 2, /* DEX idx values resolved */
	CLASS_RESOLVED = 3, /* part of linking */
	CLASS_VERIFYING = 4, /* in the process of being verified */
	CLASS_VERIFIED = 5, /* logically part of linking; done pre-init */
	CLASS_INITIALIZING = 6, /* class init in progress */
	CLASS_INITIALIZED = 7, /* ready to go */
}typedef ClassStatus;

typedef struct ClassObject {
	struct Object o; // emulate C++ inheritance, Collin

	/* leave space for instance data; we could access fields directly if we
	 freeze the definition of java/lang/Class */
	u4 instanceData[4];

	/* UTF-8 descriptor for the class; from constant pool, or on heap
	 if generated ("[C") */
	const char* descriptor;
	char* descriptorAlloc;

	/* access flags; low 16 bits are defined by VM spec */
	u4 accessFlags;

	/* VM-unique class serial number, nonzero, set very early */
	u4 serialNumber;

	/* DexFile from which we came; needed to resolve constant pool entries */
	/* (will be NULL for VM-generated, e.g. arrays and primitive classes) */
	void* pDvmDex;

	/* state of class initialization */
	ClassStatus status;

	/* if class verify fails, we must return same error on subsequent tries */
	struct ClassObject* verifyErrorClass;

	/* threadId, used to check for recursive <clinit> invocation */
	u4 initThreadId;

	/*
	 * Total object size; used when allocating storage on gc heap.  (For
	 * interfaces and abstract classes this will be zero.)
	 */
	size_t objectSize;

	/* arrays only: class object for base element, for instanceof/checkcast
	 (for String[][][], this will be String) */
	struct ClassObject* elementClass;

	/* arrays only: number of dimensions, e.g. int[][] is 2 */
	int arrayDim;
	PrimitiveType primitiveType;

	/* superclass, or NULL if this is java.lang.Object */
	struct ClassObject* super;

	/* defining class loader, or NULL for the "bootstrap" system loader */
	struct Object* classLoader;

	struct InitiatingLoaderList initiatingLoaderList;

	/* array of interfaces this class implements directly */
	int interfaceCount;
	struct ClassObject** interfaces;

	/* static, private, and <init> methods */
	int directMethodCount;
	struct Method* directMethods;

	/* virtual methods defined in this class; invoked through vtable */
	int virtualMethodCount;
	struct Method* virtualMethods;

	/*
	 * Virtual method table (vtable), for use by "invoke-virtual".  The
	 * vtable from the superclass is copied in, and virtual methods from
	 * our class either replace those from the super or are appended.
	 */
	int vtableCount;
	struct Method** vtable;

} ClassObject;

typedef struct Method {
	struct ClassObject *clazz;
	u4 accessFlags;

	u2 methodIndex;

	u2 registersSize; /* ins + locals */
	u2 outsSize;
	u2 insSize;

	/* method name, e.g. "<init>" or "eatLunch" */
	const char* name;

	/*
	 * Method prototype descriptor string (return and argument types).
	 *
	 * TODO: This currently must specify the DexFile as well as the proto_ids
	 * index, because generated Proxy classes don't have a DexFile.  We can
	 * remove the DexFile* and reduce the size of this struct if we generate
	 * a DEX for proxies.
	 */
	DexProto prototype;

	/* short-form method descriptor string */
	const char* shorty;

	/*
	 * The remaining items are not used for abstract or native methods.
	 * (JNI is currently hijacking "insns" as a function pointer, set
	 * after the first call.  For internal-native this stays null.)
	 */

	/* the actual code */
	u2* insns;

	/* cached JNI argument and return-type hints */
	int jniArgInfo;

	/*
	 * Native method ptr; could be actual function or a JNI bridge.  We
	 * don't currently discriminate between DalvikBridgeFunc and
	 * DalvikNativeFunc; the former takes an argument superset (i.e. two
	 * extra args) which will be ignored.  If necessary we can use
	 * insns==NULL to detect JNI bridge vs. internal native.
	 */
	DalvikBridgeFunc nativeFunc;

#ifdef WITH_PROFILER
	bool inProfile;
#endif
#ifdef WITH_DEBUGGER
	short debugBreakpointCount;
#endif

	bool fastJni;

	/*
	 * JNI: true if this method has no reference arguments. This lets the JNI
	 * bridge avoid scanning the shorty for direct pointers that need to be
	 * converted to local references.
	 *
	 * TODO: replace this with a list of indexes of the reference arguments.
	 */
	bool noRef;

} Method;

/*
 * Array objects have these additional fields.
 *
 * We don't currently store the size of each element.  Usually it's implied
 * by the instruction.  If necessary, the width can be derived from
 * the first char of obj->clazz->name.
 */
typedef struct ArrayObject {
	Object obj; /* MUST be first item */

	/* number of elements; immutable after init */
	u4 length;

	/*
	 * Array contents; actual size is (length * sizeof(type)).  This is
	 * declared as u8 so that the compiler inserts any necessary padding
	 * (e.g. for EABI); the actual allocation may be smaller than 8 bytes.
	 */
	u8 contents[1];
} ArrayObject;

typedef void (*DalvikNativeFunc)(const u4* args, jvalue* pResult);

typedef struct DalvikNativeMethod_t {
	const char* name;
	const char* signature;
	DalvikNativeFunc fnPtr;
} DalvikNativeMethod;

/* flags for dvmMalloc */
enum {
	ALLOC_DEFAULT = 0x00, ALLOC_DONT_TRACK = 0x01, /* don't add to internal tracking list */
	ALLOC_NON_MOVING = 0x02,
};

static void dalvik_dispatcher(const u4* args, jvalue* pResult,
		const Method* method, void* self);
static void* dvm_dlsym(void *hand, const char *name);
static ArrayObject* boxMethodArgs(const Method* method, const u4* args);
static void throwNPE(JNIEnv* env, const char* msg);
static s8 dvmGetArgLong(const u4* args, int elem);
static bool dvmIsStaticMethod(const Method* method);
static bool dvmIsPrimitiveClass(const ClassObject* clazz);

//typedef void* (*dvmIsStaticMethod_func)(void*);

typedef int (*dvmComputeMethodArgsSize_func)(void*);
typedef void (*dvmCallMethod_func)(void*, const Method*, void*, void*, void*,
		...);

typedef size_t (*dexProtoGetParameterCount_func)(const DexProto *);
typedef ArrayObject* (*dvmAllocArrayByClass_func)(void*, size_t, int);
typedef void* (*dvmBoxPrimitive_func)(jvalue, void*);
typedef void* (*dvmFindPrimitiveClass_func)(const char);
typedef void (*dvmReleaseTrackedAlloc_func)(void*, void*);
typedef ClassObject* (*dvmFindArrayClass_func)(const char*, void*);
//typedef jlong (*dvmGetArgLong_func)(const u4*,int);
typedef int (*dvmCheckException_func)(void*);
typedef Object* (*dvmGetException_func)(void*);
typedef Object* (*dvmCreateReflectMethodObject_func)(const Method*);

typedef ClassObject* (*dvmGetBoxedReturnType_func)(const Method*);
//typedef int (*dvmIsPrimitiveClass_func)(ClassObject*);
typedef int (*dvmUnboxPrimitive_func)(void*, ClassObject*, void*);
typedef Object* (*dvmDecodeIndirectRef_func)(void* self, jobject jobj);
typedef void* (*dvmThreadSelf_func)();

dvmComputeMethodArgsSize_func dvmComputeMethodArgsSize_fnPtr;
//dvmIsStaticMethod_func dvmIsStaticMethod_fnPtr;
dvmCallMethod_func dvmCallMethod_fnPtr;

dexProtoGetParameterCount_func dexProtoGetParameterCount_fnPtr;
dvmAllocArrayByClass_func dvmAllocArrayByClass_fnPtr;
dvmBoxPrimitive_func dvmBoxPrimitive_fnPtr;
dvmFindPrimitiveClass_func dvmFindPrimitiveClass_fnPtr;
dvmReleaseTrackedAlloc_func dvmReleaseTrackedAlloc_fnPtr;
dvmFindArrayClass_func dvmFindArrayClass_fnPtr;
//dvmGetArgLong_func dvmGetArgLong_fnPtr;
dvmCheckException_func dvmCheckException_fnPtr;
dvmGetException_func dvmGetException_fnPtr;
dvmCreateReflectMethodObject_func dvmCreateReflectMethodObject_fnPtr;

dvmGetBoxedReturnType_func dvmGetBoxedReturnType_fnPtr;
//dvmIsPrimitiveClass_func dvmIsPrimitiveClass_fnPtr;
dvmUnboxPrimitive_func dvmUnboxPrimitive_fnPtr;
dvmDecodeIndirectRef_func dvmDecodeIndirectRef_fnPtr;
dvmThreadSelf_func dvmThreadSelf_fnPtr;
