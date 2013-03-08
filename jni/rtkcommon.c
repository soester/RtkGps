
#include <android/log.h>
#include <jni.h>
#include <strings.h>

#include "rtklib.h"

#define TAG "nativeRtkCommion"
#define LOGV(...) showmsg(__VA_ARGS__)

static jobject RtkCommon_get_sat_id(JNIEnv* env, jclass clazz, jint sat_no)
{
   char sat_id[16];
   satno2id(sat_no, sat_id);
   return (*env)->NewStringUTF(env, sat_id);
}

static void RtkCommon_dops(JNIEnv* env, jclass clazz, jdoubleArray j_azel, jint j_ns, jdouble j_elmin, jobject j_dst)
{
   static jmethodID set_dops_mid = NULL;
   double dst[4];
   double *azel;

   azel = (*env)->GetDoubleArrayElements(env, j_azel, NULL);
   if (azel == NULL)
      return;

   dops(j_ns, azel, j_elmin, dst);

   (*env)->ReleaseDoubleArrayElements(env, j_azel, azel, 0);

   if (set_dops_mid == NULL) {
      set_dops_mid = (*env)->GetMethodID(env,
	    (*env)->GetObjectClass(env, j_dst),
	    "setDops",
	    "(DDDD)V");
      if (set_dops_mid == NULL) {
	 LOGV("setDops() not found");
	 return;
      }
   }

   (*env)->CallVoidMethod(env, j_dst, set_dops_mid,
	 dst[0], dst[1], dst[2], dst[3]);
}

static jdouble RtkCommon_geoidh(JNIEnv* env, jclass clazz, jdouble j_lat, jdouble j_lon)
{
   double pos[2] = {j_lat, j_lon};
   return (jdouble)geoidh(pos);
}


static void RtkCommon__ecef2pos(JNIEnv* env, jclass clazz, jdouble x,
      jdouble y, jdouble z, jdoubleArray j_pos)
{
   double r[3] = {x, y, z};
   double pos[3];

   ecef2pos(r, pos);
   (*env)->SetDoubleArrayRegion(env, j_pos, 0, 3, pos);
}

static jdouble RtkCommon__ecef2enu(JNIEnv* env, jclass clazz, jdouble j_lat,
      jdouble j_lon, jdoubleArray j_r, jdoubleArray j_e)
{
   double pos[2] = {j_lat, j_lon};
   double r[3];
   double e[3];

   (*env)->GetDoubleArrayRegion(env, j_r, 0, 3, r);
   if ((*env)->ExceptionOccurred(env) != NULL)
      return;
   ecef2enu(pos, r, e);
   (*env)->SetDoubleArrayRegion(env, j_e, 0, 3, e);
}


static void RtkCommon__covenu(JNIEnv* env, jclass clazz, jdouble j_lat,
      jdouble j_lon, jdoubleArray j_P, jdoubleArray j_Q )
{
   double pos[2] = {j_lat, j_lon};
   double P[9];
   double Q[9];

   (*env)->GetDoubleArrayRegion(env, j_P, 0, 9, P);
   if ((*env)->ExceptionOccurred(env) != NULL)
      return;
   covenu(pos, P, Q);
   (*env)->SetDoubleArrayRegion(env, j_Q, 0, 9, Q);
}

static void RtkCommon__deg2dms(JNIEnv* env, jclass clazz, jdouble j_deg,
      jdoubleArray j_dst)
{
   double dms[3];
   deg2dms(j_deg, dms);
   (*env)->SetDoubleArrayRegion(env, j_dst, 0, 3, dms);
}

static jdouble RtkCommon_norm(JNIEnv* env, jclass clazz, jdoubleArray j_a)
{
   jsize size;
   double res;
   double *a;
   size = (*env)->GetArrayLength(env, j_a);
   if (size == 0)
      return 0.0;

   a = (*env)->GetDoubleArrayElements(env, j_a, NULL);
   if (a == NULL)
      return 0.0;
   res = norm(a, size);
   (*env)->ReleaseDoubleArrayElements(env, j_a, a, 0);

   return res;
}

static JNINativeMethod nativeMethods[] = {
   {"getSatId", "(I)Ljava/lang/String;", (void*)RtkCommon_get_sat_id},
   {"dops", "([DIDLru0xdc/rtklib/Dops;)V", (void*)RtkCommon_dops},
   {"geoidh", "(DD)D", (void*)RtkCommon_geoidh},
   {"_deg2dms", "(D[D)V", (void*)RtkCommon__deg2dms},
   {"norm", "([D)D", (void*)RtkCommon_norm},
   {"_ecef2pos", "(DDD[D)V", (void*)RtkCommon__ecef2pos},
   {"_ecef2enu", "(DD[D[D)V", (void*)RtkCommon__ecef2enu},
   {"_covenu", "(DD[D[D)V", (void*)RtkCommon__covenu},
};

int registerRtkCommonNatives(JNIEnv* env) {
    int result = -1;

    /* look up the class */
    jclass clazz = (*env)->FindClass(env, "ru0xdc/rtklib/RtkCommon");

    if (clazz == NULL)
       return JNI_FALSE;

    if ((*env)->RegisterNatives(env, clazz, nativeMethods, sizeof(nativeMethods)
	     / sizeof(nativeMethods[0])) != JNI_OK)
       return JNI_FALSE;

    return JNI_TRUE;
}

