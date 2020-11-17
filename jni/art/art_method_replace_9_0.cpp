/*
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
 * art_method_replace_9_0.cpp
 * Created by Tanck 11/14/2020 11:31PM
 */

#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#include <ctime>
#include <cstdlib>
#include <cstddef>
#include <cassert>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cctype>
#include <cerrno>
#include <utime.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "art.h"
#include "art_9_0.h"
#include "../common.h"

/**
 * Replace the method on android9
 * @param env the native env
 * @param src the bug method
 * @param dest the fixed method
 */
void replace_9_0(JNIEnv *env, jobject src, jobject dest) {
    auto *source_method = (art::mirror::ArtMethod *) env->FromReflectedMethod(src);
    auto *dest_method   = (art::mirror::ArtMethod *) env->FromReflectedMethod(dest);

    reinterpret_cast<art::mirror::Class *>(dest_method->declaring_class_)->clinit_thread_id_ = reinterpret_cast<art::mirror::Class *>(source_method->declaring_class_)->clinit_thread_id_;
    reinterpret_cast<art::mirror::Class *>(dest_method->declaring_class_)->status_           = reinterpret_cast<art::mirror::Class *>(source_method->declaring_class_)->status_ - 1;
    reinterpret_cast<art::mirror::Class *>(dest_method->declaring_class_)->super_class_      = 0;

    source_method->declaring_class_                                        = dest_method->declaring_class_;
    source_method->access_flags_                                           = dest_method->access_flags_ | 0x0001;
    source_method->dex_code_item_offset_                                   = dest_method->dex_code_item_offset_;
    source_method->dex_method_index_                                       = dest_method->dex_method_index_;
    source_method->method_index_                                           = dest_method->method_index_;
    source_method->hotness_count_                                          = dest_method->hotness_count_;
    source_method->ptr_sized_fields_.data_                                 = dest_method->ptr_sized_fields_.data_;
    source_method->ptr_sized_fields_.entry_point_from_quick_compiled_code_ = dest_method->ptr_sized_fields_.entry_point_from_quick_compiled_code_;

    LOGD("replace_9_0: %d , %d",
         source_method->ptr_sized_fields_.entry_point_from_quick_compiled_code_,
         dest_method->ptr_sized_fields_.entry_point_from_quick_compiled_code_);

}

void setFieldFlag_9_0(JNIEnv *env, jobject field) {
    auto *artField = (art::mirror::ArtField *) env->FromReflectedField(field);
    artField->access_flags_ = artField->access_flags_ & (~0x0002) | 0x0001;
    LOGD("setFieldFlag_9_0: %d ", artField->access_flags_);
}