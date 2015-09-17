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
 * 	art_method_replace_5_0.cpp
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

#include "art.h"
#include "art_5_0.h"
#include "common.h"

void replace_5_0(JNIEnv* env, jobject src, jobject dest) {
	art::mirror::ArtMethod* smeth =
			(art::mirror::ArtMethod*) env->FromReflectedMethod(src);

	art::mirror::ArtMethod* dmeth =
			(art::mirror::ArtMethod*) env->FromReflectedMethod(dest);

	dmeth->declaring_class_->class_loader_ =
			smeth->declaring_class_->class_loader_; //for plugin classloader
	dmeth->declaring_class_->clinit_thread_id_ =
			smeth->declaring_class_->clinit_thread_id_;
	dmeth->declaring_class_->status_ = smeth->declaring_class_->status_-1;

	smeth->declaring_class_ = dmeth->declaring_class_;
	smeth->access_flags_ = dmeth->access_flags_;
	smeth->frame_size_in_bytes_ = dmeth->frame_size_in_bytes_;
	smeth->dex_cache_initialized_static_storage_ =
			dmeth->dex_cache_initialized_static_storage_;
	smeth->dex_cache_resolved_types_ = dmeth->dex_cache_resolved_types_;
	smeth->dex_cache_resolved_methods_ = dmeth->dex_cache_resolved_methods_;
	smeth->vmap_table_ = dmeth->vmap_table_;
	smeth->core_spill_mask_ = dmeth->core_spill_mask_;
	smeth->fp_spill_mask_ = dmeth->fp_spill_mask_;
	smeth->mapping_table_ = dmeth->mapping_table_;
	smeth->code_item_offset_ = dmeth->code_item_offset_;
	smeth->entry_point_from_compiled_code_ =
			dmeth->entry_point_from_compiled_code_;

	smeth->entry_point_from_interpreter_ = dmeth->entry_point_from_interpreter_;
	smeth->native_method_ = dmeth->native_method_;
	smeth->method_index_ = dmeth->method_index_;
	smeth->method_dex_index_ = dmeth->method_dex_index_;

	LOGD("replace_5_0: %d , %d", smeth->entry_point_from_compiled_code_,
			dmeth->entry_point_from_compiled_code_);

}

void setFieldFlag_5_0(JNIEnv* env, jobject field) {
	art::mirror::ArtField* artField =
			(art::mirror::ArtField*) env->FromReflectedField(field);
	artField->access_flags_ = artField->access_flags_ & (~0x0002) | 0x0001;
	LOGD("setFieldFlag_5_0: %d ", artField->access_flags_);
}

