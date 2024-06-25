/**
* @file main.cpp
* @brief Command line compiler for LSL
*
* $LicenseInfo:firstyear=2024&license=viewerlgpl$
* Kyler "Félix" Eastridge
* Copyright (C) 2024, Alchemy Viewer Project.
* Copyright (C) 2010, Linden Research, Inc.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation;
* version 2.1 of the License only.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*
* $/LicenseInfo$
*/

#include "library.h"
#include "linden_common.h"
#include "lscript_library.h"
#include "llrand.h"
#include <ctime>
#include <lltimer.h>

void llSin(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = sin(args[0].mFP);
}

void llCos(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = cos(args[0].mFP);
}

void llTan(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = tan(args[0].mFP);
}

void llAtan2(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = atan2(args[0].mFP, args[1].mFP);
}

void llSqrt(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = sqrt(args[0].mFP);
}

void llPow(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = pow(args[0].mFP, args[1].mFP);
}

void llAbs(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    retval->mInteger = abs(args[0].mInteger);
}

void llFabs(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = fabs(args[0].mFP);
}

void llFrand(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = ll_frand(args[0].mFP);
}

void llFloor(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    retval->mInteger = llfloor(args[0].mFP);
}

void llCeil(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    retval->mInteger = llceil(args[0].mFP);
}

void llRound(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    retval->mInteger = ll_round(args[0].mFP);
}

void llVecMag(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = args[0].mVec.magVec();
}

void llVecNorm(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    args[0].mVec.normalize();
    retval->mVec = args[0].mVec;
}

void llVecDist(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = dist_vec(args[0].mVec, args[1].mVec);
}

void llRot2Euler(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    args[0].mQuat.getEulerAngles(&retval->mVec.mV[VX], &retval->mVec.mV[VY], &retval->mVec.mV[VZ]);
}

void llEuler2Rot(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_QUATERNION;
    retval->mQuat.setEulerAngles(args[0].mVec.mV[VX], args[0].mVec.mV[VY], args[0].mVec.mV[VZ]);
}

void llAxes2Rot(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_QUATERNION;
    retval->mQuat = LLQuaternion(args[0].mVec, args[1].mVec, args[2].mVec);
}

void llRot2Fwd(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    retval->mVec = LLVector3(1, 0, 0) * args[0].mQuat;
    retval->mVec.normalize();
}

void llRot2Left(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    retval->mVec = LLVector3(0, 1, 0) * args[0].mQuat;
    retval->mVec.normalize();
}

void llRot2Up(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    retval->mVec = LLVector3(0, 0, 1) * args[0].mQuat;
    retval->mVec.normalize();
}

void llRotBetween(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_QUATERNION;
    retval->mQuat.shortestArc(args[0].mVec, args[1].mVec);
}

void llGetWallclock(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    struct tm* time = utc_to_pacific_time(time_corrected(), is_daylight_savings());
    retval->mFP = time->tm_hour * 3600 + time->tm_min * 60 + time->tm_sec;
}

void llGetSubString(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    //TEST: Ensure this works!
    retval->mType = LST_STRING;
    const char *sourceStr = args[0].mString;
    int strLen = strlen(sourceStr);
    int start = args[1].mInteger;
    int end = args[2].mInteger;

    // Handle negative indices
    if (start < 0) start += strLen;
    if (end < 0) end += strLen;

    // Ensure start and end are within bounds
    if (start < 0) start = 0;
    if (end >= strLen) end = strLen - 1;

    // Determine the behavior based on the comparison of start and end
    if (start <= end)
    {
        // Normal substring extraction
        int length = end - start + 1;
        retval->mString = new char[length + 1];
        strncpy(retval->mString, sourceStr + start, length);
        retval->mString[length] = '\0'; // Null-terminate the string
    }
    else
    {
        // Exclusion case: create a string excluding the range from end to start
        int length = strLen - (start - end - 1);
        retval->mString = new char[length + 1];
        strncpy(retval->mString, sourceStr, end + 1);
        strcpy(retval->mString + end + 1, sourceStr + start);
        // Don't need to \0 the string here because we copied that from the other
    }
}

void llDeleteSubString(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;
    const char *sourceStr = args[0].mString;
    int strLen = strlen(sourceStr);
    int start = args[1].mInteger;
    int end = args[2].mInteger;

    // Handle negative indices
    if (start < 0) start += strLen;
    if (end < 0) end += strLen;

    // Ensure start and end are within bounds
    if (start < 0) start = 0;
    if (end >= strLen) end = strLen - 1;

    if (start <= end)
    {
        // Normal deletion of characters from start to end
        int length = strLen - (end - start - 1);
        retval->mString = new char[length + 1];
        // Copy from the start of the string to the 'start' position
        strncpy(retval->mString, sourceStr, start);
        // Copy from the 'end + 1' position to the end of the string
        strcpy(retval->mString + start, sourceStr + end + 1);
        retval->mString[length] = '\0'; // Null-terminate the string
    }
    else
    {
        //FIXME: This doesn't work properly!
        // Exclusion case: create a string excluding the range from end to start
        int length = strLen - (end + 1) - (strLen - start);
        retval->mString = new char[length + 1];

        // Copy from the 'start' position to the end of the string
        strcpy(retval->mString, sourceStr + start - 3);

        // Copy from the start of the string to the 'end' position
        strncpy(retval->mString + (strLen - start), sourceStr, end);

        retval->mString[length] = '\0'; // Null-terminate the string
    }
}

void llInsertString(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;
    int newSize = strlen(args[0].mString) + strlen(args[2].mString);
    retval->mString = new char[newSize];
    int insertPos = args[1].mInteger;

    // Bounds check
    if (insertPos < 0)
        insertPos = 0;
    else if (insertPos > strlen(args[0].mString))
        insertPos = strlen(args[0].mString);

    int stringPos = 0;
    //Copy the existing text
    strncpy(retval->mString, args[0].mString, insertPos);
    stringPos =+ insertPos;

    //Copy the new text
    strcpy(retval->mString + stringPos, args[2].mString);
    stringPos += strlen(args[2].mString);

    //Copy the remainder of text
    strcpy(retval->mString + stringPos, args[0].mString + insertPos);
}

void llToUpper(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;
    retval->mString = new char[strlen(args[0].mString)];
    strcpy(retval->mString, args[0].mString);
    for(int i=0, l=strlen(retval->mString); i<l; i++)
    {
        retval->mString[i] = toupper(retval->mString[i]);
    }
}

void llToLower(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;
    retval->mString = new char[strlen(args[0].mString)];
    strcpy(retval->mString, args[0].mString);
    for(int i=0, l=strlen(retval->mString); i<l; i++)
    {
        retval->mString[i] = tolower(retval->mString[i]);
    }
}

//TODO: Test me
void llAxisAngle2Rot(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_QUATERNION;
    retval->mQuat = LLQuaternion(args[1].mFP, args[0].mVec);
}

//TODO: Test me
void llRot2Axis(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    F32 angle_radians, x, y, z;
    args[0].mQuat.getAngleAxis(&angle_radians, &x, &y, &z);
    retval->mVec = LLVector3(x, y, z);
    retval->mVec.normalize();
}

//TODO: Test me
void llRot2Angle(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    F32 angle_radians, x, y, z;
    args[0].mQuat.getAngleAxis(&angle_radians, &x, &y, &z);
    retval->mFP = angle_radians;
}

//TODO: Test me
void llAcos(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = acos(args[0].mFP);
}

//TODO: Test me
void llAsin(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = asin(args[0].mFP);
}

void llStringLength(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    retval->mInteger = strlen(args[0].mString);
}

void LSLLibrary::init()
{
    mLibrary.assignExec("llSin", llSin);
    mLibrary.assignExec("llCos", llCos);
    mLibrary.assignExec("llTan", llTan);
    mLibrary.assignExec("llAtan2", llAtan2);
    mLibrary.assignExec("llSqrt", llSqrt);
    mLibrary.assignExec("llPow", llPow);
    mLibrary.assignExec("llAbs", llAbs);
    mLibrary.assignExec("llFabs", llFabs);
    mLibrary.assignExec("llFrand", llFrand);
    mLibrary.assignExec("llFloor", llFloor);
    mLibrary.assignExec("llCeil", llCeil);
    mLibrary.assignExec("llRound", llRound);
    mLibrary.assignExec("llVecMag", llVecMag);
    mLibrary.assignExec("llVecNorm", llVecNorm);
    mLibrary.assignExec("llVecDist", llVecDist);
    mLibrary.assignExec("llRot2Euler", llRot2Euler);
    mLibrary.assignExec("llEuler2Rot", llEuler2Rot);
    mLibrary.assignExec("llAxes2Rot", llAxes2Rot);
    mLibrary.assignExec("llRot2Fwd", llRot2Fwd);
    mLibrary.assignExec("llRot2Left", llRot2Left);
    mLibrary.assignExec("llRot2Up", llRot2Up);
    mLibrary.assignExec("llRotBetween", llRotBetween);

    mLibrary.assignExec("llGetWallclock", llGetWallclock);

    mLibrary.assignExec("llGetSubString", llGetSubString);
    mLibrary.assignExec("llDeleteSubString", llDeleteSubString);
    mLibrary.assignExec("llInsertString", llInsertString);

    mLibrary.assignExec("llToUpper", llToUpper);
    mLibrary.assignExec("llToLower", llToLower);

    mLibrary.assignExec("llAxisAngle2Rot", llAxisAngle2Rot);
    mLibrary.assignExec("llRot2Axis", llRot2Axis);
    mLibrary.assignExec("llRot2Angle", llRot2Angle);
    mLibrary.assignExec("llAcos", llAcos);
    mLibrary.assignExec("llAsin", llAsin);

/*
    mLibrary.assignExec("llAngleBetween", "f", "qq");

    mLibrary.assignExec("llSubStringIndex", "i", "ss");

    mLibrary.assignExec("llListSort", "l", "lii");
    mLibrary.assignExec("llGetListLength", "i", "l");
    mLibrary.assignExec("llList2Integer", "i", "li");
    mLibrary.assignExec("llList2Float", "f", "li");
    mLibrary.assignExec("llList2String", "s", "li");
    mLibrary.assignExec("llList2Key", "k", "li");
    mLibrary.assignExec("llList2Vector", "v", "li");
    mLibrary.assignExec("llList2Rot", "q", "li");
    mLibrary.assignExec("llList2List", "l", "lii");
    mLibrary.assignExec("llDeleteSubList", "l", "lii");
    mLibrary.assignExec("llGetListEntryType", "i", "li");
    mLibrary.assignExec("llList2CSV", "s", "l");
    mLibrary.assignExec("llCSV2List", "l", "s");
    mLibrary.assignExec("llListRandomize", "l", "li");
    mLibrary.assignExec("llList2ListStrided", "l", "liii");

    mLibrary.assignExec("llListInsertList", "l", "lli");
    mLibrary.assignExec("llListFindList", "i", "ll");

    mLibrary.assignExec("llGetDate", "s", NULL);

    mLibrary.assignExec("llParseString2List", "l", "sll");

    mLibrary.assignExec("llGetFreeMemory", "i", NULL);

    mLibrary.assignExec("llDumpList2String", "s", "ls");

    mLibrary.assignExec("llMD5String", "s", "si");

    mLibrary.assignExec("llStringToBase64", "s", "s");
    mLibrary.assignExec("llBase64ToString", "s", "s");
    mLibrary.assignExec( "llXorBase64Strings", "s", "ss");

    mLibrary.assignExec("llLog10", "f", "f");
    mLibrary.assignExec("llLog", "f", "f");

    mLibrary.assignExec("llGetTimestamp", "s", NULL);

    mLibrary.assignExec( "llIntegerToBase64", "s", "i");
    mLibrary.assignExec( "llBase64ToInteger", "i", "s");
    mLibrary.assignExec("llGetGMTclock", "f", "");

    mLibrary.assignExec("llParseStringKeepNulls", "l", "sll");

    mLibrary.assignExec("llListReplaceList", "l", "llii");

    mLibrary.assignExec("llModPow", "i", "iii");

    mLibrary.assignExec("llEscapeURL", "s", "s");
    mLibrary.assignExec("llUnescapeURL", "s", "s");

    mLibrary.assignExec("llListStatistics", "f", "il");
    mLibrary.assignExec("llGetUnixTime", "i", NULL);

    mLibrary.assignExec("llXorBase64StringsCorrect", "s", "ss");

    mLibrary.assignExec("llStringTrim", "s", "si");

    mLibrary.assignExec("llSHA1String", "s", "s");
*/
}
