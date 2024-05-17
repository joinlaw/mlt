/*
 * factory.c -- the factory method interfaces
 * Copyright (C) 2024 Meltytech, LLC
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

#include <framework/mlt.h>

#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lv2.h>

/* lv2 extenstions */
#include <lv2/atom/atom.h>
#include <lv2/midi/midi.h>
#include <lv2/port-groups/port-groups.h>
#include <lv2/port-props/port-props.h>
#include <lv2/presets/presets.h>
#include <lv2/resize-port/resize-port.h>
#include <lv2/ui/ui.h>
#include <lv2/worker/worker.h>

#include <lilv/lilv.h>

#include "lv2_mgr.h"
#include <ladspa.h>

extern mlt_filter filter_lv2_init(mlt_profile profile,
				      mlt_service_type type,
				      const char *id,
				      char *arg);
extern mlt_producer producer_lv2_init(mlt_profile profile,
                                         mlt_service_type type,
                                         const char *id,
                                         char *arg);

lv2_mgr_t *g_lv2_plugin_mgr = NULL;

static void add_port_to_metadata(mlt_properties p, lv2_plugin_desc_t *desc, int j)
{
    LADSPA_PortRangeHintDescriptor hint_descriptor = desc->port_range_hints[j].HintDescriptor;

    mlt_properties_set(p, "title", desc->port_names[j]);
    if (LADSPA_IS_HINT_INTEGER(hint_descriptor)) {
      mlt_properties_set(p, "type", "integer");
      mlt_properties_set_int(p, 
			     "default",
			     (int) desc->def_values[j]);
      mlt_properties_set_double(p,
				"minimum",
				(int) desc->min_values[j]);
      mlt_properties_set_double(p,
				"maximum",
				(int) desc->max_values[j]);
    } else if (LADSPA_IS_HINT_TOGGLED(hint_descriptor)) {
      mlt_properties_set(p, "type", "boolean");
      mlt_properties_set_int(p,
			     "default",
			     desc->def_values[j]);
    } else {
      mlt_properties_set(p, "type", "float");
      mlt_properties_set_double(p,
				"default",
				desc->def_values[j]);
      mlt_properties_set_double(p,
				"minimum",
				desc->min_values[j]);
      mlt_properties_set_double(p,
				"maximum",
				desc->max_values[j]);
	
    }

    if (LADSPA_IS_HINT_ENUMERATION(hint_descriptor))
      {
	/* WIP */
      }

    if (LADSPA_IS_HINT_LOGARITHMIC(hint_descriptor))
        mlt_properties_set(p, "scale", "log");
    mlt_properties_set(p, "mutable", "yes");
    mlt_properties_set(p, "animation", "yes");
}

static mlt_properties metadata(mlt_service_type type, const char *id, char *data)
{
    char file[PATH_MAX];
    if (type == mlt_service_filter_type) {

        snprintf(file,
                 PATH_MAX,
                 "%s/lv2/%s",
                 mlt_environment("MLT_DATA"),
		 strncmp(id, "lv2.", 4) ? data : "filter_lv2.yml");
    } else {
        snprintf(file,
                 PATH_MAX,
                 "%s/lv2/%s",
                 mlt_environment("MLT_DATA"),
		 strncmp(id, "lv2.", 4) ? data : "producer_lv2.yml");
    }
    mlt_properties result = mlt_properties_parse_yaml(file);

    if (!strncmp(id, "lv2.", 4)) {
        // Annotate the yaml properties with lv2 control port info.
      lv2_plugin_desc_t *desc = lv2_mgr_get_any_desc(g_lv2_plugin_mgr, (char *) &id[4]);

        if (desc) {

            mlt_properties params = mlt_properties_new();
            mlt_properties p;
            char key[20];
            int i;

            mlt_properties_set(result, "identifier", id);
            mlt_properties_set(result, "title", desc->name);
            mlt_properties_set(result, "creator", desc->maker ? desc->maker : "unknown");
            mlt_properties_set(result, "description", "LADSPA plugin");
            mlt_properties_set_data(result,
                                    "parameters",
                                    params,
                                    0,
                                    (mlt_destructor) mlt_properties_close,
                                    NULL);
            for (i = 0; i < desc->control_port_count; i++) {
                int j = desc->control_port_indicies[i];
                p = mlt_properties_new();
                snprintf(key, sizeof(key), "%d", mlt_properties_count(params));
                mlt_properties_set_data(params,
                                        key,
                                        p,
                                        0,
                                        (mlt_destructor) mlt_properties_close,
                                        NULL);
                snprintf(key, sizeof(key), "%d", j);
                mlt_properties_set(p, "identifier", key);
                add_port_to_metadata(p, desc, j);
                mlt_properties_set(p, "mutable", "yes");
            }
            for (i = 0; i < desc->status_port_count; i++) {
                int j = desc->status_port_indicies[i];
                p = mlt_properties_new();
                snprintf(key, sizeof(key), "%d", mlt_properties_count(params));
                mlt_properties_set_data(params,
                                        key,
                                        p,
                                        0,
                                        (mlt_destructor) mlt_properties_close,
                                        NULL);
                snprintf(key, sizeof(key), "%d[*]", j);
                mlt_properties_set(p, "identifier", key);
                add_port_to_metadata(p, desc, j);
                mlt_properties_set(p, "readonly", "yes");
            }

            p = mlt_properties_new();
            snprintf(key, sizeof(key), "%d", mlt_properties_count(params));
            mlt_properties_set_data(params, key, p, 0, (mlt_destructor) mlt_properties_close, NULL);
            mlt_properties_set(p, "identifier", "instances");
            mlt_properties_set(p, "title", "Instances");
            mlt_properties_set(p,
                               "description",
                               "The number of instances of the plugin that are in use.\n"
                               "MLT will create the number of plugins that are required "
                               "to support the number of audio channels.\n"
                               "Status parameters (readonly) are provided for each instance "
                               "and are accessed by specifying the instance number after the "
                               "identifier (starting at zero).\n"
                               "e.g. 9[0] provides the value of status 9 for the first instance.");
            mlt_properties_set(p, "type", "integer");
            mlt_properties_set(p, "readonly", "yes");

            if (type == mlt_service_filter_type) {

                p = mlt_properties_new();
                snprintf(key, sizeof(key), "%d", mlt_properties_count(params));
                mlt_properties_set_data(params,
                                        key,
                                        p,
                                        0,
                                        (mlt_destructor) mlt_properties_close,
                                        NULL);
                mlt_properties_set(p, "identifier", "wetness");
                mlt_properties_set(p, "title", "Wet/Dry");
                mlt_properties_set(p, "type", "float");
                mlt_properties_set_double(p, "default", 1);
                mlt_properties_set_double(p, "minimum", 0);
                mlt_properties_set_double(p, "maximum", 1);
                mlt_properties_set(p, "mutable", "yes");
                mlt_properties_set(p, "animation", "yes");
            }
        }
    }

    return result;
}

MLT_REPOSITORY
{

    GSList *list;

    g_lv2_plugin_mgr = lv2_mgr_new();

    for (list = g_lv2_plugin_mgr->all_plugins; list; list = g_slist_next(list)) {

      lv2_plugin_desc_t *desc = (lv2_plugin_desc_t *) list->data;
      char *s = NULL;
      s = calloc(1, strlen("lv2.") + strlen (desc->uri) + 1);

      sprintf(s, "lv2.%s", desc->uri);

      char *str_ptr = strchr(s, ':');
      while (str_ptr != NULL)
	{
	  *str_ptr++ = '<';
	  str_ptr = strchr(str_ptr, ':');
	}
      
      if (desc->has_input) {
	MLT_REGISTER(mlt_service_filter_type, s, filter_lv2_init);
	MLT_REGISTER_METADATA(mlt_service_filter_type, s, metadata, NULL);
      } else {
	MLT_REGISTER(mlt_service_producer_type, s, producer_lv2_init);
	MLT_REGISTER_METADATA(mlt_service_producer_type, s, metadata, NULL);
      }

      if (s)
	{
	  free(s);
	}
    }

    mlt_factory_register_for_clean_up(g_lv2_plugin_mgr, (mlt_destructor) lv2_mgr_destroy);

    MLT_REGISTER(mlt_service_filter_type, "lv2", filter_lv2_init);
    MLT_REGISTER_METADATA(mlt_service_filter_type, "lv2", metadata, "filter_lv2.yml");
}
