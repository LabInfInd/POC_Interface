#pragma once

#include <QObject>

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

#include <Window3dWrapper.h>

using namespace std;
using namespace nlohmann;

class ProcessWorker  : public QObject
{
	Q_OBJECT
private:
	// Global State and Key Process Function
	bool s_isRunning = true;
	Visualization::Layout3d s_layoutMode = Visualization::Layout3d::OnlyMainView;
	bool s_visualizeJointFrame = false;


	bool predict_joints(json&, int, k4abt_tracker_t, k4a_capture_t, Window3dWrapper&, int, int);

	bool check_depth_image_exists(k4a_capture_t);

public:
	ProcessWorker(QObject *parent);
	~ProcessWorker();

public slots:
	void process_mkv_offline(const char*, const char*, k4abt_tracker_configuration_t&);

signals:
	void resultReady(bool result);
};
