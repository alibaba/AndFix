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

package com.euler.test;

import android.util.Log;

/**
 * new version of {@link A}
 * 
 * @author sanping.li@alipay.com
 * 
 */
public class Fix {
	String s = "s";
	private static O o = new O("fix");
	static int i = 11;

	public static String a(String str) {
		Log.d("qq", str);
		Log.i("euler", "fix succes");
		return "b";
	}

	public int b(String s1, String s2) {
		Log.e("euler", s1);
		Log.e("euler", i + "==" + c());
		Log.i("euler", "fix succes" + s);
		Log.i("euler", o.s);
		return 0;
	}

	private String c() {
		return "a";
	}
}
