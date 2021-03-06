#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include <QMultiMap>
#include <QDebug>
#include <QFile>
#include "math.h"

#define VERSION 1.0

#define EVENT_SECONDS_FOR_START -5
#define EVENT_DURATION 120
#define SECNODS_FOR_ALIGN_QSCD 10
#define PI 3.14159265358979323846
#define DATA_DURATION 86400
#define MAX_NUM_STATION 300

#define MAX_NUM_EVENT 20

#define STA_LEN 10
#define CHAN_LEN 5
#define NET_LEN 4
#define LOC_LEN 4

#define IMAGE_X_WIDTH 814
#define IMAGE_Y_HEIGHT 768

#define P_VEL 6.5
#define S_VEL 3.5

#define LANDXYCNT 127822

typedef struct _station
{
    int index;
    char netsta[STA_LEN];
    float lat;
    float lon;
    int mapX;
    int mapY;
    int lastPGATime;
    float pga;
} _STATION;

typedef struct _event
{
    int evid;        // same with eew_evid
    int eventEpochStartTime;
    float lat;
    float lon;
    int mapX;
    int mapY;
    float mag;
} _EVENT;

typedef struct _binary_station_packet
{
    int numPGAsta;
    int dataTime;
    _STATION staList[MAX_NUM_STATION];
} _BINARY_STATION_PACKET;

typedef struct _binary_point_packet
{
    int dataTime;
    float mapZ[LANDXYCNT];
} _BINARY_POINT_PACKET;

typedef struct _binary_eew_packet
{
    int numEVENT;
    _EVENT eventlist[MAX_NUM_EVENT];
} _BINARY_EEW_PACKET;

typedef struct _point
{
    int landX;
    int landY;
} _POINT;

#endif // COMMON_H
