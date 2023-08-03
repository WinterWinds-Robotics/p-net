/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2021 rt-labs AB, Sweden.
 *
 * This software is dual-licensed under GPLv3 and a commercial
 * license. See the file LICENSE.md distributed with this software for
 * full license information.
 ********************************************************************/

#include "app_data.h"
#include "app_utils.h"
#include "app_gsdml.h"
#include "app_log.h"
#include "sampleapp_common.h"
#include "osal.h"
#include "pnal.h"
#include <pnet_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define APP_DATA_DEFAULT_OUTPUT_DATA 0

/* Parameter data for digital submodules
 * The stored value is shared between all digital submodules in this example.
 *
 * Todo: Data is always in pnio data format. Add conversion to uint32_t.
 */
static uint32_t app_param_1 = 0; /* Network endianness */
static uint32_t app_param_2 = 0; /* Network endianness */

/* Parameter data for echo submodules
 * The stored value is shared between all echo submodules in this example.
 *
 * Todo: Data is always in pnio data format. Add conversion to uint32_t.
 */
static uint32_t app_param_echo_gain = 1; /* Network endianness */

/* Digital submodule process data
 * The stored value is shared between all digital submodules in this example. */
static uint8_t inputdata[APP_GSDML_INPUT_DATA_DIGITAL_SIZE] = {0};
static uint8_t outputdata[APP_GSDML_OUTPUT_DATA_DIGITAL_SIZE] = {0};
static uint8_t counter = 0;

/* Network endianness */
static uint8_t echo_inputdata[APP_GSDML_INPUT_DATA_ECHO_SIZE] = {0};
static uint8_t echo_outputdata[APP_GSDML_OUTPUT_DATA_ECHO_SIZE] = {0};

CC_PACKED_BEGIN
typedef struct CC_PACKED app_echo_data
{
   /* Network endianness.
      Used as a float, but we model it as a 4-byte integer to easily
      do endianness conversion */
   uint32_t echo_float_bytes;

   /* Network endianness */
   uint32_t echo_int;
} app_echo_data_t;
CC_PACKED_END
CC_STATIC_ASSERT (sizeof (app_echo_data_t) == APP_GSDML_INPUT_DATA_ECHO_SIZE);
CC_STATIC_ASSERT (sizeof (app_echo_data_t) == APP_GSDML_OUTPUT_DATA_ECHO_SIZE);

/**
 * Set LED state.
 *
 * Compares new state with previous state, to minimize system calls.
 *
 * Uses the hardware specific app_set_led() function.
 *
 * @param led_state        In:    New LED state
 */
static void app_handle_data_led_state (bool led_state)
{
   static bool previous_led_state = false;

   if (led_state != previous_led_state)
   {
      app_set_led (APP_DATA_LED_ID, led_state);
   }
   previous_led_state = led_state;
}
#define ARRAY_SIZE(arr) (sizeof (arr) / sizeof ((arr)[0]))

int fgetfloat(const char *fpath, float *rv) {
    int rc = 0;
    *rv = -999.999;
    rc = access(fpath, F_OK);
    if( rc != 0 ) {
       fprintf(stderr, "ERROR: Unable to access() %s", fpath);
    } else {
        FILE * fp = fopen (fpath, "r");
        if (fp) {
            char buf[32];
            int n = fread (buf, sizeof (*buf), ARRAY_SIZE (buf), fp);
            if (!n) {
               rc = 1;
               fprintf(stderr, "uh oh\n");
            } else {
               *rv = strtof (buf, NULL);
            }
         fclose (fp);
        }
    }
    return rc;
}


uint8_t * app_data_get_input_data (
   uint16_t slot_nbr,
   uint16_t subslot_nbr,
   uint32_t submodule_id,
   bool button_pressed,
   uint16_t * size,
   uint8_t * iops)
{
   static unsigned long int seq = 0;
   static float inputfloat = 2.0;
   float outputfloat;
   uint32_t hostorder_inputfloat_bytes;
   uint32_t hostorder_outputfloat_bytes;
   printf (
      "%s: seq:%lu,  slot_nbr:%x, sub_nbr: %x, sub_id:%x\n",
      __FUNCTION__,
      seq++,
      slot_nbr,
      subslot_nbr,
      submodule_id);
   app_echo_data_t * p_echo_inputdata = (app_echo_data_t *)&echo_inputdata;
   app_echo_data_t * p_echo_outputdata = (app_echo_data_t *)&echo_outputdata;

   if (size == NULL || iops == NULL)
   {
      return NULL;
   }
    if( submodule_id > APP_GSDML_SUBMOD_ID_IEDSTATS_MIN 
          && submodule_id < APP_GSDML_SUBMOD_ID_IEDSTATS_MAX) {
        int rc;
        switch(submodule_id) {
            case APP_GSDML_SUBMOD_ID_IEDSTATS_APPCOUNT:
                rc = fgetfloat("/tmp/zdata/ApplicationCount", &inputfloat);
                break;
            case APP_GSDML_SUBMOD_ID_IEDSTATS_MEMPCT:
                rc = fgetfloat("/tmp/zdata/MemoryPercentage", &inputfloat);
                break;
            case APP_GSDML_SUBMOD_ID_IEDSTATS_STORPCT:
                rc = fgetfloat("/tmp/zdata/DataSpacePercentage", &inputfloat);
                break;
            case APP_GSDML_SUBMOD_ID_IEDSTATS_MEM:
                rc = fgetfloat("/tmp/zdata/MemoryUsed", &inputfloat);
                break;
            case APP_GSDML_SUBMOD_ID_IEDSTATS_CPUPCT:
                rc = fgetfloat("/tmp/zdata/CpuPercentage", &inputfloat);
                break;
            default:
                rc = 1;
                break;
        }
        if(rc != 0 ) {
           fprintf(stderr, "fgetfloat returned %d\n", rc);
        }
        printf ("inputfloat: %f\n", inputfloat);
        memcpy (&hostorder_inputfloat_bytes, &inputfloat, sizeof (outputfloat));
        p_echo_inputdata->echo_float_bytes =
         CC_TO_BE32 (hostorder_inputfloat_bytes);

        *size = APP_GSDML_INPUT_DATA_ECHO_SIZE;
        *iops = PNET_IOXS_GOOD;
        return echo_inputdata;

    }
      if (submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_IN)
   {
      inputdata[0] = 0xff;
      /*  FILE * fp = fopen ("input", "r");
        if (fp) {
           char buf[4];
           int rc = fread(buf, sizeof(*buf), ARRAY_SIZE(buf), fp);
           if( !rc ) {
                   printf("uh oh\n");
           }
           unsigned long int x = strtoul(buf,NULL,16);
           inputdata[0] = (unsigned char)(x&0xff);
           fclose (fp);
           printf("\ninputdata: %08x\n",( unsigned int) x);
        } */
      *size = APP_GSDML_INPUT_DATA_DIGITAL_SIZE;
      *iops = PNET_IOXS_GOOD;
      return inputdata;
   }
   else if (submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_IN_OUT)
   {
      /* Prepare digital input data
       * Lowest 7 bits: Counter    Most significant bit: Button
       */

      counter = counter + 12345;
      inputdata[0] = counter++;
      if (button_pressed)
      {
         inputdata[0] |= 0x80;
      }
      else
      {
         inputdata[0] &= 0x7F;
      }

      *size = APP_GSDML_INPUT_DATA_DIGITAL_SIZE;
      *iops = PNET_IOXS_GOOD;
      return inputdata;
   }

   if (submodule_id == APP_GSDML_SUBMOD_ID_ECHO)
   {
      /* Calculate echodata input (to the PLC)
       * by multiplying the output (from the PLC) with a gain factor
       */

      /* Integer */
      p_echo_inputdata->echo_int = CC_TO_BE32 (
         CC_FROM_BE32 (p_echo_outputdata->echo_int) *
         CC_FROM_BE32 (app_param_echo_gain));

      /* Float */
      /* Use memcopy to avoid strict-aliasing rule warnings */
      hostorder_outputfloat_bytes =
         CC_FROM_BE32 (p_echo_outputdata->echo_float_bytes);
      memcpy (&outputfloat, &hostorder_outputfloat_bytes, sizeof (outputfloat));
      // inputfloat = outputfloat * CC_FROM_BE32 (app_param_echo_gain);

      FILE * fp = fopen ("/tmp/zdata/ApplicationCount", "r");
      if (fp)
      {
         char buf[8];
         int rc = fread (buf, sizeof (*buf), ARRAY_SIZE (buf), fp);
         if (!rc)
         {
            printf ("uh oh\n");
         }
         float x = strtof (buf, NULL);
         inputfloat = x;
         fclose (fp);
      }
      // inputfloat += 2.25;
      printf ("inputfloat: %f\n", inputfloat);
      memcpy (&hostorder_inputfloat_bytes, &inputfloat, sizeof (outputfloat));
      p_echo_inputdata->echo_float_bytes =
         CC_TO_BE32 (hostorder_inputfloat_bytes);

      *size = APP_GSDML_INPUT_DATA_ECHO_SIZE;
      *iops = PNET_IOXS_GOOD;
      return echo_inputdata;
   }

   /* Automated RT Tester scenario 2 - unsupported (sub)module */
   return NULL;
}

int app_data_set_output_data (
   uint16_t slot_nbr,
   uint16_t subslot_nbr,
   uint32_t submodule_id,
   uint8_t * data,
   uint16_t size)
{
   bool led_state;
   // static unsigned long int seq = 0;

   /*
   printf (
      "%s: %lu: %u %u %u -->|%s|<--\n",
      __FUNCTION__,
      seq++,
      slot_nbr,
      subslot_nbr,
      submodule_id,
      data);
      */
   if (data == NULL)
   {
      return -1;
   }

   if (
      submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_OUT ||
      submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_IN_OUT)
   {
      if (size == APP_GSDML_OUTPUT_DATA_DIGITAL_SIZE)
      {
         memcpy (outputdata, data, size);

         /* Most significant bit: LED */
         led_state = (outputdata[0] & 0x80) > 0;
         app_handle_data_led_state (led_state);

         return 0;
      }
   }
   else if (submodule_id == APP_GSDML_SUBMOD_ID_ECHO)
   {
      if (size == APP_GSDML_OUTPUT_DATA_ECHO_SIZE)
      {
         memcpy (echo_outputdata, data, size);

         return 0;
      }
   }

   return -1;
}

int app_data_set_default_outputs (void)
{
   outputdata[0] = APP_DATA_DEFAULT_OUTPUT_DATA;
   app_handle_data_led_state (false);
   return 0;
}

int app_data_write_parameter (
   uint16_t slot_nbr,
   uint16_t subslot_nbr,
   uint32_t submodule_id,
   uint32_t index,
   const uint8_t * data,
   uint16_t length)
{
   const app_gsdml_param_t * par_cfg;

   par_cfg = app_gsdml_get_parameter_cfg (submodule_id, index);
   if (par_cfg == NULL)
   {
      APP_LOG_WARNING (
         "PLC write request unsupported submodule/parameter. "
         "Submodule id: %u Index: %u\n",
         (unsigned)submodule_id,
         (unsigned)index);
      return -1;
   }

   if (length != par_cfg->length)
   {
      APP_LOG_WARNING (
         "PLC write request unsupported length. "
         "Index: %u Length: %u Expected length: %u\n",
         (unsigned)index,
         (unsigned)length,
         par_cfg->length);
      return -1;
   }

   if (index == APP_GSDML_PARAMETER_1_IDX)
   {
      memcpy (&app_param_1, data, length);
   }
   else if (index == APP_GSDML_PARAMETER_2_IDX)
   {
      memcpy (&app_param_2, data, length);
   }
   else if (index == APP_GSDML_PARAMETER_ECHO_IDX)
   {
      memcpy (&app_param_echo_gain, data, length);
   }

   APP_LOG_DEBUG ("  Writing parameter \"%s\"\n", par_cfg->name);
   app_log_print_bytes (APP_LOG_LEVEL_DEBUG, data, length);

   return 0;
}

int app_data_read_parameter (
   uint16_t slot_nbr,
   uint16_t subslot_nbr,
   uint32_t submodule_id,
   uint32_t index,
   uint8_t ** data,
   uint16_t * length)
{
   const app_gsdml_param_t * par_cfg;

   par_cfg = app_gsdml_get_parameter_cfg (submodule_id, index);
   if (par_cfg == NULL)
   {
      APP_LOG_WARNING (
         "PLC read request unsupported submodule/parameter. "
         "Submodule id: %u Index: %u\n",
         (unsigned)submodule_id,
         (unsigned)index);
      return -1;
   }

   if (*length < par_cfg->length)
   {
      APP_LOG_WARNING (
         "PLC read request unsupported length. "
         "Index: %u Max length: %u Data length for our parameter: %u\n",
         (unsigned)index,
         (unsigned)*length,
         par_cfg->length);
      return -1;
   }

   APP_LOG_DEBUG ("  Reading \"%s\"\n", par_cfg->name);
   if (index == APP_GSDML_PARAMETER_1_IDX)
   {
      *data = (uint8_t *)&app_param_1;
      *length = sizeof (app_param_1);
   }
   else if (index == APP_GSDML_PARAMETER_2_IDX)
   {
      *data = (uint8_t *)&app_param_2;
      *length = sizeof (app_param_2);
   }
   else if (index == APP_GSDML_PARAMETER_ECHO_IDX)
   {
      *data = (uint8_t *)&app_param_echo_gain;
      *length = sizeof (app_param_echo_gain);
   }

   app_log_print_bytes (APP_LOG_LEVEL_DEBUG, *data, *length);

   return 0;
}
