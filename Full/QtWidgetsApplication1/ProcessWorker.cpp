#include "ProcessWorker.h"

bool ProcessWorker::predict_joints(json& frames_json, int frame_count, k4abt_tracker_t tracker, k4a_capture_t capture_handle, Window3dWrapper& window3d, int depthWidth, int depthHeight)
{
    k4a_wait_result_t queue_capture_result = k4abt_tracker_enqueue_capture(tracker, capture_handle, K4A_WAIT_INFINITE);
    if (queue_capture_result != K4A_WAIT_RESULT_SUCCEEDED)
    {
        cerr << "Error! Adding capture to tracker process queue failed!" << endl;
        return false;
    }

    k4abt_frame_t body_frame = nullptr;
    k4a_wait_result_t pop_frame_result = k4abt_tracker_pop_result(tracker, &body_frame, K4A_WAIT_INFINITE);
    if (pop_frame_result != K4A_WAIT_RESULT_SUCCEEDED)
    {
        cerr << "Error! Popping body tracking result failed!" << endl;
        return false;
    }

    uint32_t num_bodies = k4abt_frame_get_num_bodies(body_frame);
    uint64_t timestamp = k4abt_frame_get_device_timestamp_usec(body_frame);

    json frame_result_json;
    frame_result_json["timestamp_usec"] = timestamp;
    frame_result_json["frame_id"] = frame_count;
    frame_result_json["num_bodies"] = num_bodies;
    frame_result_json["bodies"] = json::array();
    for (uint32_t i = 0; i < num_bodies; i++)
    {
        k4abt_skeleton_t skeleton;
        VERIFY(k4abt_frame_get_body_skeleton(body_frame, i, &skeleton), "Get body from body frame failed!");
        json body_result_json;
        int body_id = k4abt_frame_get_body_id(body_frame, i);
        body_result_json["body_id"] = body_id;

        for (int j = 0; j < (int)K4ABT_JOINT_COUNT; j++)
        {
            body_result_json["joint_positions"].push_back({ skeleton.joints[j].position.xyz.x,
                                                                skeleton.joints[j].position.xyz.y,
                                                                skeleton.joints[j].position.xyz.z });

            body_result_json["joint_orientations"].push_back({ skeleton.joints[j].orientation.wxyz.w,
                                                                skeleton.joints[j].orientation.wxyz.x,
                                                                skeleton.joints[j].orientation.wxyz.y,
                                                                skeleton.joints[j].orientation.wxyz.z });
        }
        frame_result_json["bodies"].push_back(body_result_json);
    }
    frames_json.push_back(frame_result_json);

    //////////////////////////////


     // Obtain original capture that generates the body tracking result
    k4a_capture_t originalCapture = k4abt_frame_get_capture(body_frame);
    k4a_image_t depthImage = k4a_capture_get_depth_image(originalCapture);

    std::vector<Color> pointCloudColors(depthWidth * depthHeight, { 1.f, 1.f, 1.f, 1.f });

    // Read body index map and assign colors
    k4a_image_t bodyIndexMap = k4abt_frame_get_body_index_map(body_frame);
    const uint8_t* bodyIndexMapBuffer = k4a_image_get_buffer(bodyIndexMap);
    for (int i = 0; i < depthWidth * depthHeight; i++)
    {
        uint8_t bodyIndex = bodyIndexMapBuffer[i];
        if (bodyIndex != K4ABT_BODY_INDEX_MAP_BACKGROUND)
        {
            uint32_t bodyId = k4abt_frame_get_body_id(body_frame, bodyIndex);
            pointCloudColors[i] = g_bodyColors[bodyId % g_bodyColors.size()];
        }
    }
    k4a_image_release(bodyIndexMap);

    // Visualize point cloud
    window3d.UpdatePointClouds(depthImage, pointCloudColors);

    // Visualize the skeleton data
    window3d.CleanJointsAndBones();
    uint32_t numBodies = k4abt_frame_get_num_bodies(body_frame);
    for (uint32_t i = 0; i < numBodies; i++)
    {
        k4abt_body_t body;
        VERIFY(k4abt_frame_get_body_skeleton(body_frame, i, &body.skeleton), "Get skeleton from body frame failed!");
        body.id = k4abt_frame_get_body_id(body_frame, i);

        // Assign the correct color based on the body id
        Color color = g_bodyColors[body.id % g_bodyColors.size()];
        color.a = 0.4f;
        Color lowConfidenceColor = color;
        lowConfidenceColor.a = 0.1f;

        // Visualize joints
        for (int joint = 0; joint < static_cast<int>(K4ABT_JOINT_COUNT); joint++)
        {
            if (body.skeleton.joints[joint].confidence_level >= K4ABT_JOINT_CONFIDENCE_LOW)
            {
                const k4a_float3_t& jointPosition = body.skeleton.joints[joint].position;
                const k4a_quaternion_t& jointOrientation = body.skeleton.joints[joint].orientation;

                window3d.AddJoint(
                    jointPosition,
                    jointOrientation,
                    body.skeleton.joints[joint].confidence_level >= K4ABT_JOINT_CONFIDENCE_MEDIUM ? color : lowConfidenceColor);
            }
        }

        // Visualize bones
        for (size_t boneIdx = 0; boneIdx < g_boneList.size(); boneIdx++)
        {
            k4abt_joint_id_t joint1 = g_boneList[boneIdx].first;
            k4abt_joint_id_t joint2 = g_boneList[boneIdx].second;

            if (body.skeleton.joints[joint1].confidence_level >= K4ABT_JOINT_CONFIDENCE_LOW &&
                body.skeleton.joints[joint2].confidence_level >= K4ABT_JOINT_CONFIDENCE_LOW)
            {
                bool confidentBone = body.skeleton.joints[joint1].confidence_level >= K4ABT_JOINT_CONFIDENCE_MEDIUM &&
                    body.skeleton.joints[joint2].confidence_level >= K4ABT_JOINT_CONFIDENCE_MEDIUM;
                const k4a_float3_t& joint1Position = body.skeleton.joints[joint1].position;
                const k4a_float3_t& joint2Position = body.skeleton.joints[joint2].position;

                window3d.AddBone(joint1Position, joint2Position, confidentBone ? color : lowConfidenceColor);
            }
        }
    }

    k4a_capture_release(originalCapture);
    k4a_image_release(depthImage);

    window3d.SetLayout3d(s_layoutMode);
    window3d.SetJointFrameVisualization(s_visualizeJointFrame);
    window3d.Render();


    /////////////////////////////


    k4abt_frame_release(body_frame);

    return true;
}

bool ProcessWorker::check_depth_image_exists(k4a_capture_t capture)
{
    k4a_image_t depth = k4a_capture_get_depth_image(capture);
    if (depth != nullptr)
    {
        k4a_image_release(depth);
        return true;
    }
    else
    {
        return false;
    }
}

ProcessWorker::ProcessWorker(QObject *parent)
	: QObject(parent)
{}

ProcessWorker::~ProcessWorker()
{}

void ProcessWorker::process_mkv_offline(const char* input_path, const char* output_path, k4abt_tracker_configuration_t& tracker_config)
{
    cout << "Tutto ok;";
    k4a_playback_t playback_handle = nullptr;
    k4a_result_t result = k4a_playback_open(input_path, &playback_handle);
    if (result != K4A_RESULT_SUCCEEDED)
    {
        cerr << "Cannot open recording at " << input_path << endl;
        emit resultReady(false);
    }

    k4a_calibration_t calibration;
    result = k4a_playback_get_calibration(playback_handle, &calibration);
    if (result != K4A_RESULT_SUCCEEDED)
    {
        cerr << "Failed to get calibration" << endl;
        emit resultReady(false);
    }

    k4abt_tracker_t tracker = NULL;
    if (K4A_RESULT_SUCCEEDED != k4abt_tracker_create(&calibration, tracker_config, &tracker))
    {
        cerr << "Body tracker initialization failed!" << endl;
        emit resultReady(false);
    }

    json json_output;
    json_output["k4abt_sdk_version"] = K4ABT_VERSION_STR;
    json_output["source_file"] = input_path;

    // Store all joint names to the json
    json_output["joint_names"] = json::array();
    for (int i = 0; i < (int)K4ABT_JOINT_COUNT; i++)
    {
        json_output["joint_names"].push_back(g_jointNames.find((k4abt_joint_id_t)i)->second);
    }

    // Store all bone linkings to the json
    json_output["bone_list"] = json::array();
    for (int i = 0; i < (int)g_boneList.size(); i++)
    {
        json_output["bone_list"].push_back({ g_jointNames.find(g_boneList[i].first)->second,
                                             g_jointNames.find(g_boneList[i].second)->second });
    }

    cout << "Tracking " << input_path << endl;

    int frame_count = 0;
    json frames_json = json::array();
    bool success = true;

    int depthWidth = calibration.depth_camera_calibration.resolution_width;
    int depthHeight = calibration.depth_camera_calibration.resolution_height;

    Window3dWrapper window3d;

    window3d.Create("3D Visualization", calibration);


    while (true)
    {
        k4a_capture_t capture_handle = nullptr;
        k4a_stream_result_t stream_result = k4a_playback_get_next_capture(playback_handle, &capture_handle);
        if (stream_result == K4A_STREAM_RESULT_EOF)
        {
            break;
        }

        cout << "frame " << frame_count << '\r';
        if (stream_result == K4A_STREAM_RESULT_SUCCEEDED)
        {
            // Only try to predict joints when capture contains depth image
            if (check_depth_image_exists(capture_handle))
            {
                success = predict_joints(frames_json, frame_count, tracker, capture_handle, window3d, depthWidth, depthHeight);
                k4a_capture_release(capture_handle);
                if (!success)
                {
                    cerr << "Predict joints failed for clip at frame " << frame_count << endl;
                    break;
                }
            }
        }
        else
        {
            success = false;
            cerr << "Stream error for clip at frame " << frame_count << endl;
            break;
        }

        frame_count++;
    }

    if (success)
    {
        json_output["frames"] = frames_json;
        cout << endl << "DONE " << endl;

        cout << "Total read " << frame_count << " frames" << endl;
        std::ofstream output_file(output_path);
        output_file << std::setw(4) << json_output << std::endl;
        cout << "Results saved in " << output_path;
    }

    window3d.Delete();

    k4abt_tracker_shutdown(tracker);
    k4abt_tracker_destroy(tracker);
    k4a_playback_close(playback_handle);

    emit resultReady(success);
}
