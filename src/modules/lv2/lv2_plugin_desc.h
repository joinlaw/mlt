/*
 * JACK Rack
 *
 * Original:
 * Copyright (C) Robert Ham 2002, 2003 (node@users.sourceforge.net)
 *
 * Modification for MLT:
 * Copyright (C) 2004-2014 Meltytech, LLC
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __LV2_PLUGIN_DESC_H__
#define __LV2_PLUGIN_DESC_H__

#include <ladspa.h>
#include <glib.h>


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

typedef struct _lv2_plugin_desc lv2_plugin_desc_t;

struct _lv2_plugin_desc
{
  char *                   uri; /* file name */
  unsigned long            index;
  unsigned long            id;
  char *                   name;
  char *                   maker;
  LADSPA_Properties        properties;
  gboolean                 rt;
  
  unsigned long            channels;
  
  gboolean                 aux_are_input;
  unsigned long            aux_channels;

  unsigned long            port_count;
  LADSPA_PortDescriptor *  port_descriptors;
  LADSPA_PortRangeHint *   port_range_hints;
  char **                  port_names;
  
  unsigned long *          audio_input_port_indicies;
  unsigned long *          audio_output_port_indicies;
  
  unsigned long *          audio_aux_port_indicies;

  unsigned long            control_port_count;
  unsigned long *          control_port_indicies;

  unsigned long            status_port_count;
  unsigned long *          status_port_indicies;

  float *def_values, *min_values, *max_values;

  gboolean                 has_input;
};

lv2_plugin_desc_t * lv2_plugin_desc_new ();
lv2_plugin_desc_t * lv2_plugin_desc_new_with_descriptor (const char * uri,
						  unsigned long index,
						  const LilvPlugin * plugin);
void            lv2_plugin_desc_destroy (lv2_plugin_desc_t * pd);

void lv2_plugin_desc_set_uri (lv2_plugin_desc_t * pd, const char * uri);
void lv2_plugin_desc_set_index       (lv2_plugin_desc_t * pd, unsigned long index);
void lv2_plugin_desc_set_id          (lv2_plugin_desc_t * pd, unsigned long id);
void lv2_plugin_desc_set_name        (lv2_plugin_desc_t * pd, const char * name);
void lv2_plugin_desc_set_maker       (lv2_plugin_desc_t * pd, const char * maker);
void lv2_plugin_desc_set_properties  (lv2_plugin_desc_t * pd, LADSPA_Properties properties);

struct _lv2_plugin * lv2_plugin_desc_instantiate (lv2_plugin_desc_t * pd);

LADSPA_Data lv2_plugin_desc_change_control_value (lv2_plugin_desc_t *, unsigned long, LADSPA_Data, guint32, guint32);

gint lv2_plugin_desc_get_copies (lv2_plugin_desc_t * pd, unsigned long rack_channels);

#endif /* __LV2_PLUGIN_DESC_H__ */
