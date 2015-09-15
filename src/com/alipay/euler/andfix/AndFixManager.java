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

package com.alipay.euler.andfix;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Method;
import java.util.Enumeration;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import android.content.Context;
import android.util.Log;

import com.alipay.euler.andfix.annotation.MethodReplace;
import com.alipay.euler.andfix.security.SecurityChecker;

import dalvik.system.DexFile;

/**
 * AndFix Manager
 * 
 * @author sanping.li@alipay.com
 * 
 */
public class AndFixManager {
	private static final String TAG = "AndFixManager";

	private static final String DIR = "apatch_opt";

	/**
	 * context
	 */
	private final Context mContext;

	/**
	 * classes will be fixed
	 */
	private static Map<String, Class<?>> mFixedClass = new ConcurrentHashMap<String, Class<?>>();

	/**
	 * whether support AndFix
	 */
	private boolean mSupport = false;

	/**
	 * security check
	 */
	private SecurityChecker mSecurityChecker;

	/**
	 * optimize directory
	 */
	private File mOptDir;

	public AndFixManager(Context context) {
		mContext = context;
		mSupport = Compat.isSupport();
		if (mSupport) {
			mSecurityChecker = new SecurityChecker(mContext);
			mOptDir = new File(mContext.getFilesDir(), DIR);
			if (!mOptDir.exists() && !mOptDir.mkdirs()) {// make directory fail
				mSupport = false;
				Log.e(TAG, "opt dir create error.");
			} else if (!mOptDir.isDirectory()) {// not directory
				mOptDir.delete();
				mSupport = false;
			}
		}
	}

	/**
	 * delete optimize file of patch file
	 * 
	 * @param file
	 *            patch file
	 */
	public synchronized void removeOptFile(File file) {
		File optfile = new File(mOptDir, file.getName());
		if (optfile.exists() && !optfile.delete()) {
			Log.e(TAG, optfile.getName() + " delete error.");
		}
	}

	/**
	 * fix
	 * 
	 * @param patchPath
	 *            patch path
	 */
	public synchronized void fix(String patchPath) {
		fix(new File(patchPath), mContext.getClassLoader(), null);
	}

	/**
	 * fix
	 * 
	 * @param file
	 *            patch file
	 * @param classLoader
	 *            classloader of class that will be fixed
	 * @param classes
	 *            classes will be fixed
	 */
	public synchronized void fix(File file, ClassLoader classLoader,
			List<String> classes) {
		if (!mSupport) {
			return;
		}

		if (!mSecurityChecker.verifyApk(file)) {// security check fail
			return;
		}

		try {
			File optfile = new File(mOptDir, file.getName());
			boolean saveFingerprint = true;
			if (optfile.exists()) {
				// need to verify fingerprint when the optimize file exist,
				// prevent someone attack on jailbreak device with
				// Vulnerability-Parasyte.
				// btw:exaggerated android Vulnerability-Parasyte
				// http://secauo.com/Exaggerated-Android-Vulnerability-Parasyte.html
				if (mSecurityChecker.verifyOpt(optfile)) {
					saveFingerprint = false;
				} else if (!optfile.delete()) {
					return;
				}
			}

			final DexFile dexFile = DexFile.loadDex(file.getAbsolutePath(),
					optfile.getAbsolutePath(), Context.MODE_PRIVATE);

			if (saveFingerprint) {
				mSecurityChecker.saveOptSig(optfile);
			}

			ClassLoader patchClassLoader = new ClassLoader(classLoader) {
				@Override
				protected Class<?> findClass(String className)
						throws ClassNotFoundException {
					Class<?> clazz = dexFile.loadClass(className, this);
					if (clazz == null
							&& className.startsWith("com.alipay.euler.andfix")) {
						return Class.forName(className);// annotation’s class
														// not found
					}
					if (clazz == null) {
						throw new ClassNotFoundException(className);
					}
					return clazz;
				}
			};
			Enumeration<String> entrys = dexFile.entries();
			Class<?> clazz = null;
			while (entrys.hasMoreElements()) {
				String entry = entrys.nextElement();
				if (classes != null && !classes.contains(entry)) {
					continue;// skip, not need fix
				}
				clazz = dexFile.loadClass(entry, patchClassLoader);
				if (clazz != null) {
					fixClass(clazz, classLoader);
				}
			}
		} catch (IOException e) {
			Log.e(TAG, "pacth", e);
		}
	}

	/**
	 * fix class
	 * 
	 * @param clazz
	 *            class
	 */
	private void fixClass(Class<?> clazz, ClassLoader classLoader) {
		Method[] methods = clazz.getDeclaredMethods();
		MethodReplace methodReplace;
		String clz;
		String meth;
		for (Method method : methods) {
			methodReplace = method.getAnnotation(MethodReplace.class);
			if (methodReplace == null)
				continue;
			clz = methodReplace.clazz();
			meth = methodReplace.method();
			if (!isEmpty(clz) && !isEmpty(meth)) {
				replaceMethod(classLoader, clz, meth, method);
			}
		}
	}

	/**
	 * replace method
	 * 
	 * @param classLoader classloader
	 * @param clz class
	 * @param meth name of target method 
	 * @param method source method
	 */
	private void replaceMethod(ClassLoader classLoader, String clz,
			String meth, Method method) {
		try {
			String key = clz + "@" + classLoader.toString();
			Class<?> clazz = mFixedClass.get(key);
			if (clazz == null) {// class not load
				Class<?> clzz = classLoader.loadClass(clz);
				// initialize target class
				clazz = AndFix.initTargetClass(clzz);
			}
			if (clazz != null) {// initialize class OK
				mFixedClass.put(key, clazz);
				Method src = clazz.getDeclaredMethod(meth,
						method.getParameterTypes());
				AndFix.addReplaceMethod(src, method);
			}
		} catch (Exception e) {
			Log.e(TAG, "replaceMethod", e);
		}
	}

	private static boolean isEmpty(String string) {
		return string == null || string.length() <= 0;
	}

}
