/*
 * filter_ladspa.c -- filter audio through LADSPA plugins
 * Copyright (C) 2004-2018 Meltytech, LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <framework/mlt_filter.h>
#include <framework/mlt_frame.h>
#include <framework/mlt_log.h>
#include <framework/mlt_factory.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>
#include <string.h>

#include "lv2_rack.h"

#define BUFFER_LEN (10000)
#define MAX_SAMPLE_COUNT (4096)

static lv2_rack_t *initialise_lv2_rack(mlt_properties properties, int channels)
{
    lv2_rack_t *lv2rack = NULL;
    char *resource = mlt_properties_get(properties, "resource");
    if (!resource && mlt_properties_get(properties, "src"))
        resource = mlt_properties_get(properties, "src");

    char *plugin_id = NULL;
    plugin_id = mlt_properties_get (properties, "_pluginid");

    // Start LV2Rack
    if (resource || plugin_id) {

        // Create LV2Rack without Jack client name so that it only uses LV2
      lv2rack = lv2_rack_new(NULL, channels);
        mlt_properties_set_data(properties,
                                "lv2rack",
                                lv2rack,
                                0,
                                (mlt_destructor) lv2_rack_destroy,
                                NULL);

        if (resource)
	  // Load LV2 Rack XML file
	  lv2_rack_open_file(lv2rack, resource);
	else if (plugin_id) {
            // Load one LV2 plugin by its URI

	  char *id = plugin_id;
	  lv2_plugin_desc_t *desc = lv2_mgr_get_any_desc(lv2rack->plugin_mgr, id);

	  lv2_plugin_t *plugin;
	  if (desc && (plugin = lv2_rack_instantiate_plugin(lv2rack, desc))) {

	    plugin->enabled = TRUE;
	    process_add_plugin(lv2rack->procinfo, plugin);
	    mlt_properties_set_int(properties, "instances", plugin->copies);
	  } else {
	    //mlt_log_error(properties, "failed to load plugin %lu\n", id);
	    mlt_log_error(properties, "failed to load plugin `%s`\n", id);
	    return lv2rack;
	  }
	       
	  if (plugin && plugin->desc && plugin->copies == 0) {
	    // Calculate the number of channels that will work with this plugin
	    int request_channels = plugin->desc->channels;
	    while (request_channels < channels)
	      request_channels += plugin->desc->channels;
	       
	    if (request_channels != channels) {
	      // Try to load again with a compatible number of channels.
	      mlt_log_warning(
			      properties,
			      "Not compatible with %d channels. Requesting %d channels instead.\n",
			      channels,
			      request_channels);
	      lv2rack = initialise_lv2_rack(properties, request_channels);
	    } else {
	      mlt_log_error(properties, "Invalid plugin configuration: `%s`\n", id);
	      return lv2rack;
	    }
	  }
	       
	  if (plugin && plugin->desc && plugin->copies)
	    mlt_log_debug(properties,
			  "Plugin Initialized. Channels: %lu\tCopies: %d\tTotal: %lu\n",
			  plugin->desc->channels,
			  plugin->copies,
			  lv2rack->channels);
        }
    }
    return lv2rack;
}

/** Get the audio.
*/

static int lv2_get_audio(mlt_frame frame,
                            void **buffer,
                            mlt_audio_format *format,
                            int *frequency,
                            int *channels,
                            int *samples)
{
    int error = 0;

    // Get the filter service
    mlt_filter filter = mlt_frame_pop_audio(frame);

    // Get the filter properties
    mlt_properties filter_properties = MLT_FILTER_PROPERTIES(filter);

    // Check if the channel configuration has changed
    int prev_channels = mlt_properties_get_int(filter_properties, "_prev_channels");
    if (prev_channels != *channels) {
        if (prev_channels) {
            mlt_log_info(MLT_FILTER_SERVICE(filter),
                         "Channel configuration changed. Old: %d New: %d.\n",
                         prev_channels,
                         *channels);
            mlt_properties_set_data(filter_properties,
                                    "lv2rack",
                                    NULL,
                                    0,
                                    (mlt_destructor) NULL,
                                    NULL);
        }
        mlt_properties_set_int(filter_properties, "_prev_channels", *channels);
    }

    // Initialise LV2 if needed
    lv2_rack_t *lv2rack = mlt_properties_get_data(filter_properties, "lv2rack", NULL);
    if (lv2rack == NULL) {
        lv2_sample_rate = *frequency; // global inside lv2_rack

        lv2rack = initialise_lv2_rack(filter_properties, *channels);
    }

    char *plugin_id = NULL;
    plugin_id = mlt_properties_get (filter_properties, "_pluginid");

    if (lv2rack && lv2rack->procinfo && lv2rack->procinfo->chain && plugin_id) {


        lv2_plugin_t *plugin = lv2rack->procinfo->chain;
        LADSPA_Data value;
        int i, c;
        mlt_position position = mlt_filter_get_position(filter, frame);
        mlt_position length = mlt_filter_get_length2(filter, frame);

        // Get the producer's audio
        *format = mlt_audio_float;
        mlt_frame_get_audio(frame, buffer, format, frequency, channels, samples);

        // Resize the buffer if necessary.
        if (*channels < lv2rack->channels) {
            // Add extra channels to satisfy the plugin.
            // Extra channels in the buffer will be ignored by downstream services.
            int old_size = mlt_audio_format_size(*format, *samples, *channels);
            int new_size = mlt_audio_format_size(*format, *samples, lv2rack->channels);
            uint8_t *new_buffer = mlt_pool_alloc(new_size);
            memcpy(new_buffer, *buffer, old_size);
            // Put silence in extra channels.
            memset(new_buffer + old_size, 0, new_size - old_size);
            mlt_frame_set_audio(frame, new_buffer, *format, new_size, mlt_pool_release);
            *buffer = new_buffer;
        }

        for (i = 0; i < plugin->desc->control_port_count; i++) {
            // Apply the control port values
            char key[20];
	    value = plugin->desc->def_values[plugin->desc->control_port_indicies[i]];
	    snprintf(key, sizeof(key), "%d", (int) plugin->desc->control_port_indicies[i]);

            if (mlt_properties_get(filter_properties, key))
                value = mlt_properties_anim_get_double(filter_properties, key, position, length);
            for (c = 0; c < plugin->copies; c++)
	      {
		plugin->holders[c].control_memory[i] = value;
	      }
        }

        plugin->wet_dry_enabled = mlt_properties_get(filter_properties, "wetness") != NULL;
        if (plugin->wet_dry_enabled) {
            value = mlt_properties_anim_get_double(filter_properties, "wetness", position, length);
            for (c = 0; c < lv2rack->channels; c++)
                plugin->wet_dry_values[c] = value;
        }

        // Configure the buffers
        LADSPA_Data **input_buffers = mlt_pool_alloc(sizeof(LADSPA_Data *) * lv2rack->channels);
        LADSPA_Data **output_buffers = mlt_pool_alloc(sizeof(LADSPA_Data *) * lv2rack->channels);

        // Some plugins crash with too many frames (samples).
        // So, feed the plugin with N samples per loop iteration.
        int samples_offset = 0;
        int sample_count = MIN(*samples, MAX_SAMPLE_COUNT);
        for (i = 0; samples_offset < *samples; i++) {
            int j = 0;
            for (; j < lv2rack->channels; j++)
                output_buffers[j] = input_buffers[j] = (LADSPA_Data *) *buffer + j * (*samples)
                                                       + samples_offset;
            sample_count = MIN(*samples - samples_offset, MAX_SAMPLE_COUNT);
            // Do LV2 processing
            error = process_lv2(lv2rack->procinfo, sample_count, input_buffers, output_buffers);

            samples_offset += MAX_SAMPLE_COUNT;
        }

        mlt_pool_release(input_buffers);
        mlt_pool_release(output_buffers);

        // read the status port values
        for (i = 0; i < plugin->desc->status_port_count; i++) {
            char key[20];
            int p = plugin->desc->status_port_indicies[i];
            for (c = 0; c < plugin->copies; c++) {
                snprintf(key, sizeof(key), "%d[%d]", p, c);
                value = plugin->holders[c].status_memory[i];
                mlt_properties_set_double(filter_properties, key, value);
            }
        }
    } else {
        // Nothing to do.
        error = mlt_frame_get_audio(frame, buffer, format, frequency, channels, samples);
    }

    return error;
}

/** Filter processing.
*/

static mlt_frame filter_process(mlt_filter this, mlt_frame frame)
{
    if (mlt_frame_is_test_audio(frame) == 0) {
        mlt_frame_push_audio(frame, this);
        mlt_frame_push_audio(frame, lv2_get_audio);
    }

    return frame;
}

/** Constructor for the filter.
*/
mlt_filter filter_lv2_init(mlt_profile profile, mlt_service_type type, const char *id, char *arg)
{
  mlt_filter this = mlt_filter_new();
  /* mlt_filter this = mlt_factory_filter(profile, id, arga); */

    if (this != NULL) {
        mlt_properties properties = MLT_FILTER_PROPERTIES(this);
        this->process = filter_process;
        mlt_properties_set(properties, "resource", arg);
        if (!strncmp(id, "lv2.", 4))
	  {
	    mlt_properties_set(properties, "_pluginid", id + 4);
	  }
    }

    return this;
}
