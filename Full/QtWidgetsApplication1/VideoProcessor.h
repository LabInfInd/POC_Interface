#pragma once

// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

#include <k4a/k4a.h>
#include <k4arecord/playback.h>
#include <k4abt.h>
#include <nlohmann/json.hpp>

#include "BodyTrackingHelpers.h"
#include "Utilities.h"

using namespace std;
using namespace nlohmann;


bool predict_joints(json&, int, k4abt_tracker_t, k4a_capture_t);

bool check_depth_image_exists(k4a_capture_t);

bool process_mkv_offline(const char*, const char*, k4abt_tracker_configuration_t);
