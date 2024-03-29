/*
 * JACK Rack
 *
 * Original:
 * Copyright (C) Robert Ham 2002, 2003 (node@users.sourceforge.net)
 *
 * Modification for MLT:
 * Copyright (C) 2004-2021 Meltytech, LLC
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

#ifndef __JR_PLUGIN_H__
#define __JR_PLUGIN_H__

#include <glib.h>
#include <ladspa.h>
#include <pthread.h>
#ifdef WITH_JACK
#include <jack/jack.h>
#endif

#include "process.h"
#include "plugin_desc.h"

typedef struct _ladspa_holder ladspa_holder_t;
typedef struct _ladspa2_holder ladspa2_holder_t;
typedef struct _plugin plugin_t;
typedef struct _plugin2 plugin2_t;

struct _ladspa2_holder
{
  LilvInstance *instance;
  lff_t * ui_control_fifos;
  LADSPA_Data * control_memory;
  LADSPA_Data * status_memory;

#ifdef WITH_JACK
  jack_port_t **             aux_ports;
#endif
};

struct _ladspa_holder
{
  LilvInstance *instance2;
  LADSPA_Handle instance;
  lff_t * ui_control_fifos;
  LADSPA_Data * control_memory;
  LADSPA_Data * status_memory;

#ifdef WITH_JACK
  jack_port_t **             aux_ports;
#endif
};


struct _plugin
{
  plugin_desc_t *            desc;
  gint                       enabled;

  gint                       copies;
  /* LilvInstance* instead of LADSPA_Handle inside */
  ladspa_holder_t *          holders;
  LADSPA_Data **             audio_input_memory;
  LADSPA_Data **             audio_output_memory;
  
  gboolean                   wet_dry_enabled;
  /* 1.0 = all wet, 0.0 = all dry, 0.5 = 50% wet/50% dry */
  LADSPA_Data *              wet_dry_values;
  lff_t *                    wet_dry_fifos;
  
  plugin_t *                 next;
  plugin_t *                 prev;

  const LADSPA_Descriptor *  descriptor;
  void *                     dl_handle;
  struct _jack_rack *        jack_rack;
  
};

struct _plugin2
{
  plugin_desc_t *            desc;
  gint                       enabled;

  gint                       copies;
  /* LilvInstance* instead of LADSPA_Handle inside */
  ladspa2_holder_t *          holders;
  LADSPA_Data **             audio_input_memory;
  LADSPA_Data **             audio_output_memory;

  float *def_values, *min_values, *max_values;

  gboolean                   wet_dry_enabled;
  /* 1.0 = all wet, 0.0 = all dry, 0.5 = 50% wet/50% dry */
  LADSPA_Data *              wet_dry_values;
  lff_t *                    wet_dry_fifos;
  
  /* plugin_t *                 next;
     plugin_t *                 prev; */
  plugin2_t *                 next;
  plugin2_t *                 prev;

  LilvNode                   *lv2_plugin_uri;
  LilvPlugin                 *lv2_plugin; // instead of descriptor
  const LADSPA_Descriptor *  descriptor;
  void *                     dl_handle;
  struct _jack_rack *        jack_rack;
  
};


void       process_add_plugin            (process_info_t *, plugin2_t *plugin);
plugin2_t * process_remove_plugin         (process_info_t *, plugin2_t *plugin);
void       process_ablise_plugin         (process_info_t *, plugin_t *plugin, gboolean able);
void       process_ablise_plugin_wet_dry (process_info_t *, plugin_t *plugin, gboolean enable);
void       process_move_plugin           (process_info_t *, plugin2_t *plugin, gint up);
plugin2_t * process_change_plugin         (process_info_t *, plugin2_t *plugin, plugin2_t * new_plugin);

struct _jack_rack;
struct _ui;

plugin_t * plugin_new (plugin_desc_t * plugin_desc, struct _jack_rack * jack_rack);
plugin2_t * plugin2_new (plugin_desc_t * plugin_desc, struct _jack_rack * jack_rack);
void       plugin_destroy (plugin2_t * plugin);

void plugin_connect_input_ports (plugin2_t * plugin, LADSPA_Data ** inputs);
void plugin_connect_output_ports (plugin2_t * plugin);
void plugin2_connect_input_ports (plugin2_t * plugin, LADSPA_Data ** inputs);
void plugin2_connect_output_ports (plugin2_t * plugin);


#endif /* __JR_PLUGIN_H__ */
