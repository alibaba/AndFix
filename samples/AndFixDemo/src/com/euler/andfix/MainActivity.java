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

package com.euler.andfix;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import com.euler.test.A;

/**
 * sample activity
 * 
 * @author luohou
 * @author sanping.li@alipay.com
 * 
 */
public class MainActivity extends Activity {
	private static final String TAG = "euler";

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		Log.e(TAG, A.a("good"));
		Log.e(TAG, "" + new A().b("s1", "s2"));
		Log.e(TAG, "" + new A().getI());
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		android.os.Process.killProcess(android.os.Process.myPid());
	}

}
