/*
 * Copyright (C) 2021 Paul Davis <paul@linuxaudiosystems.com>
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/stock.h>

#include "pbd/compose.h"
#include "pbd/convert.h"

#include "ardour/region.h"
#include "ardour/triggerbox.h"

#include "widgets/ardour_button.h"
#include "widgets/slider_controller.h"

#include "gtkmm2ext/utils.h"

#include "audio_region_properties_box.h"
#include "midi_region_properties_box.h"
#include "audio_region_operations_box.h"
#include "midi_region_operations_box.h"
#include "slot_properties_box.h"
#include "midi_region_trimmer_box.h"

#include "ardour_ui.h"
#include "gui_thread.h"
#include "trigger_ui.h"
#include "public_editor.h"
#include "ui_config.h"
#include "utils.h"

#include "pbd/i18n.h"

using namespace ARDOUR;
using namespace ArdourWidgets;
using namespace Gtkmm2ext;
using namespace PBD;
using namespace Temporal;

static std::vector<std::string> follow_strings;
static std::string longest_follow;
static std::vector<std::string> quantize_strings;
static std::string longest_quantize;
static std::vector<std::string> launch_strings;
static std::string longest_launch;

TriggerUI::TriggerUI () :
	_follow_percent_adjustment(0,100,1)
{
	trigger = NULL;

	using namespace Gtk::Menu_Helpers;

	if (follow_strings.empty()) {
		follow_strings.push_back (follow_action_to_string (Trigger::Stop));
		follow_strings.push_back (follow_action_to_string (Trigger::Again));
		follow_strings.push_back (follow_action_to_string (Trigger::QueuedTrigger));
		follow_strings.push_back (follow_action_to_string (Trigger::NextTrigger));
		follow_strings.push_back (follow_action_to_string (Trigger::PrevTrigger));
		follow_strings.push_back (follow_action_to_string (Trigger::FirstTrigger));
		follow_strings.push_back (follow_action_to_string (Trigger::LastTrigger));
		follow_strings.push_back (follow_action_to_string (Trigger::AnyTrigger));
		follow_strings.push_back (follow_action_to_string (Trigger::OtherTrigger));

		for (std::vector<std::string>::const_iterator i = follow_strings.begin(); i != follow_strings.end(); ++i) {
			if (i->length() > longest_follow.length()) {
				longest_follow = *i;
			}
		}

		launch_strings.push_back (launch_style_to_string (Trigger::OneShot));
		launch_strings.push_back (launch_style_to_string (Trigger::Gate));
		launch_strings.push_back (launch_style_to_string (Trigger::Toggle));
		launch_strings.push_back (launch_style_to_string (Trigger::Repeat));

		for (std::vector<std::string>::const_iterator i = launch_strings.begin(); i != launch_strings.end(); ++i) {
			if (i->length() > longest_launch.length()) {
				longest_launch = *i;
			}
		}
	}

//	set_fill_color (UIConfiguration::instance().color (X_("theme:bg")));
//	name = "triggerUI-table";
//	set_row_spacing (2);
	set_spacings (2);
//	set_padding (2);
	set_homogeneous (false);

	_follow_action_button = new ArdourButton (ArdourButton::led_default_elements);
	_follow_action_button->set_name("FollowAction");
	_follow_action_button->set_text (_("Follow Action"));
//	_follow_action_button->set_active_color (UIConfiguration::instance().color ("alert:greenish"));
	_follow_action_button->signal_event().connect (sigc::mem_fun (*this, (&TriggerUI::follow_action_button_event)));

		_follow_percent_adjustment.set_lower (0.0);
		_follow_percent_adjustment.set_upper (100.0);
		_follow_percent_adjustment.set_step_increment (1.0);
		_follow_percent_adjustment.set_page_increment (5.0);

	_follow_percent_slider = new HSliderController (&_follow_percent_adjustment, boost::shared_ptr<PBD::Controllable>(), 24/*length*/, 12/*girth*/ );
	_follow_percent_slider->set_name("FollowAction");
	_follow_percent_slider->set_text (_("100% Left"));
//	_follow_percent_slider->set_active_color (UIConfiguration::instance().color ("alert:greenish"));
//	_follow_percent_slider->signal_event().connect (sigc::mem_fun (*this, (&TriggerUI::follow_action_button_event)));

	_follow_left = new ArdourDropdown;
	_follow_left->set_name("FollowAction");
	_follow_left->append_text_item (_("None"));
	_follow_left->append_text_item (_("Repeat"));
	_follow_left->append_text_item (_("Next"));
	_follow_left->append_text_item (_("Previous"));
	_follow_left->set_sizing_text (longest_follow);

	_follow_right = new ArdourDropdown;
	_follow_right->set_name("FollowAction");
	_follow_right->append_text_item (_("None"));
	_follow_right->append_text_item (_("Repeat"));
	_follow_right->append_text_item (_("Next"));
	_follow_right->append_text_item (_("Previous"));
	_follow_right->set_sizing_text (longest_follow);

	_launch_style_button = new ArdourDropdown();
	_launch_style_button->set_name("FollowAction");
	_launch_style_button->set_sizing_text (longest_launch);
	_launch_style_button->AddMenuElem (MenuElem (launch_style_to_string (Trigger::OneShot), sigc::bind (sigc::mem_fun (*this, &TriggerUI::set_launch_style), Trigger::OneShot)));
	_launch_style_button->AddMenuElem (MenuElem (launch_style_to_string (Trigger::Gate), sigc::bind (sigc::mem_fun (*this, &TriggerUI::set_launch_style), Trigger::Gate)));
	_launch_style_button->AddMenuElem (MenuElem (launch_style_to_string (Trigger::Toggle), sigc::bind (sigc::mem_fun (*this, &TriggerUI::set_launch_style), Trigger::Toggle)));
	_launch_style_button->AddMenuElem (MenuElem (launch_style_to_string (Trigger::Repeat), sigc::bind (sigc::mem_fun (*this, &TriggerUI::set_launch_style), Trigger::Repeat)));

	_legato_button = new ArdourButton(ArdourButton::led_default_elements);
	_launch_style_button->set_name("FollowAction");
	_legato_button->set_text (_("Legato"));
	_legato_button->set_active_color (UIConfiguration::instance().color ("alert:greenish"));
	_legato_button->signal_event().connect (sigc::mem_fun (*this, (&TriggerUI::legato_button_event)));

	_quantize_button = new ArdourDropdown;

#define quantize_item(b) _quantize_button->AddMenuElem (MenuElem (quantize_length_to_string (b), sigc::bind (sigc::mem_fun (*this, &TriggerUI::set_quantize), b)));

	quantize_item (BBT_Offset (0, 0, 0));
	quantize_item (BBT_Offset (0, 1, 0));
	quantize_item (BBT_Offset (0, 2, 0));
	quantize_item (BBT_Offset (0, 4, 0));
	quantize_item (BBT_Offset (0, 0, Temporal::ticks_per_beat/2));
	quantize_item (BBT_Offset (0, 0, Temporal::ticks_per_beat/4));
	quantize_item (BBT_Offset (0, 0, Temporal::ticks_per_beat/8));
	quantize_item (BBT_Offset (0, 0, Temporal::ticks_per_beat/16));

	for (std::vector<std::string>::const_iterator i = quantize_strings.begin(); i != quantize_strings.end(); ++i) {
		if (i->length() > longest_quantize.length()) {
			longest_quantize = *i;
		}
	}
	_quantize_button->set_sizing_text (longest_quantize);
	_quantize_button->set_name("FollowAction");

#undef quantize_item

	int row=0;
	Gtk::Label *label;

	label = manage(new Gtk::Label(_("Velocity Sense:")));  label->set_alignment(1.0, 0.5);
	attach(*label,                 0, 1, row, row+1, Gtk::FILL, Gtk::SHRINK ); 
	label = manage(new Gtk::Label(_("100%")));  label->set_alignment(0.0, 0.5);
	attach(*label,                 1, 2, row, row+1, Gtk::FILL, Gtk::SHRINK ); row++;

	label = manage(new Gtk::Label(_("Launch Style:")));  label->set_alignment(1.0, 0.5);
	attach(*label,                 0, 1, row, row+1, Gtk::FILL, Gtk::SHRINK );
	attach(*_launch_style_button,   1, 2, row, row+1, Gtk::FILL, Gtk::SHRINK ); row++;

	label = manage(new Gtk::Label(_("Launch Quantize:")));  label->set_alignment(1.0, 0.5);
	attach(*label,            0, 1, row, row+1, Gtk::FILL, Gtk::SHRINK );
	attach(*_quantize_button,  1, 2, row, row+1, Gtk::FILL, Gtk::SHRINK ); row++;

	label = manage(new Gtk::Label(_("Legato Mode:")));  label->set_alignment(1.0, 0.5);
	attach(*label,          0, 1, row, row+1, Gtk::FILL, Gtk::SHRINK );
	attach(*_legato_button,  1, 2, row, row+1, Gtk::FILL, Gtk::SHRINK ); row++;

	attach(*_follow_action_button,   0, 2, row, row+1, Gtk::FILL, Gtk::SHRINK ); row++;
	attach(*_follow_percent_slider,  0, 2, row, row+1, Gtk::FILL, Gtk::SHRINK ); row++;
	attach(*_follow_left,   0, 1, row, row+1, Gtk::FILL, Gtk::SHRINK );
	attach(*_follow_right,  1, 2, row, row+1, Gtk::FILL, Gtk::SHRINK ); row++;
}

TriggerUI::~TriggerUI ()
{
}

void
TriggerUI::set_trigger (ARDOUR::Trigger* t)
{
	trigger = t;

	PropertyChange pc;

	pc.add (Properties::use_follow);
	pc.add (Properties::legato);
	pc.add (Properties::quantization);
	pc.add (Properties::launch_style);
	pc.add (Properties::follow_action0);
	pc.add (Properties::follow_action1);

	trigger_changed (pc);

	trigger->PropertyChanged.connect (trigger_connections, invalidator (*this), boost::bind (&TriggerUI::trigger_changed, this, _1), gui_context());
}


void
TriggerUI::set_quantize (BBT_Offset bbo)
{
	if (bbo == BBT_Offset (0, 0, 0)) {
		/* use grid */
		bbo = BBT_Offset (1, 2, 3); /* XXX get grid from editor */
	}

	trigger->set_quantization (bbo);
}

bool
TriggerUI::follow_action_button_event (GdkEvent* ev)
{
	switch (ev->type) {
	case GDK_BUTTON_PRESS:
		trigger->set_use_follow (!trigger->use_follow());
		return true;

	default:
		break;
	}

	return false;
}

bool
TriggerUI::legato_button_event (GdkEvent* ev)
{
	switch (ev->type) {
	case GDK_BUTTON_PRESS:
		trigger->set_legato (!trigger->legato());
		return true;

	default:
		break;
	}

	return false;
}

void
TriggerUI::set_launch_style (Trigger::LaunchStyle ls)
{
	trigger->set_launch_style (ls);
}

std::string
TriggerUI::launch_style_to_string (Trigger::LaunchStyle ls)
{
	switch (ls) {
	case Trigger::OneShot:
		return _("One Shot");
	case Trigger::Gate:
		return _("Gate");
	case Trigger::Toggle:
		return _("Toggle");
	case Trigger::Repeat:
		return _("Repeat");
	}
	/*NOTREACHED*/
	return std::string();
}

std::string
TriggerUI::quantize_length_to_string (BBT_Offset const & ql)
{
	if (ql == BBT_Offset (0, 1, 0)) {
		return _("1/4");
	} else if (ql == BBT_Offset (0, 2, 0)) {
		return _("1/2");
	} else if (ql == BBT_Offset (0, 4, 0)) {
		return _("Whole");
	} else if (ql == BBT_Offset (0, 0,Temporal::ticks_per_beat/2)) {
		return _("1/8");
	} else if (ql == BBT_Offset (0, 0,Temporal::ticks_per_beat/4)) {
		return _("1/16");
	} else if (ql == BBT_Offset (0, 0,Temporal::ticks_per_beat/8)) {
		return _("1/32");
	} else if (ql == BBT_Offset (0, 0,Temporal::ticks_per_beat/16)) {
		return _("1/64");
	} else {
		return "???";
	}
}

std::string
TriggerUI::follow_action_to_string (Trigger::FollowAction fa)
{
	switch (fa) {
	case Trigger::None:
		return _("None");
	case Trigger::Stop:
		return _("Stop");
	case Trigger::Again:
		return _("Again");
	case Trigger::QueuedTrigger:
		return _("Queued");
	case Trigger::NextTrigger:
		return _("Next");
	case Trigger::PrevTrigger:
		return _("Prev");
	case Trigger::FirstTrigger:
		return _("First");
	case Trigger::LastTrigger:
		return _("Last");
	case Trigger::AnyTrigger:
		return _("Any");
	case Trigger::OtherTrigger:
		return _("Other");
	}
	/*NOTREACHED*/
	return std::string();
}

void
TriggerUI::trigger_changed (PropertyChange pc)
{
	if (pc.contains (Properties::quantization)) {
		BBT_Offset bbo (trigger->quantization());
		_quantize_button->set_active (quantize_length_to_string (bbo));
		std::cerr << "\n\n !!! quantize is " << quantize_length_to_string (bbo) << std::endl << std::endl;
	}

	if (pc.contains (Properties::use_follow)) {
		_follow_action_button->set_active_state (trigger->use_follow() ? Gtkmm2ext::ExplicitActive : Gtkmm2ext::Off);
	}

	if (pc.contains (Properties::legato)) {
		_legato_button->set_active_state (trigger->legato() ? Gtkmm2ext::ExplicitActive : Gtkmm2ext::Off);
	}

	if (pc.contains (Properties::launch_style)) {
		_launch_style_button->set_active (launch_style_to_string (trigger->launch_style()));
	}

	if (pc.contains (Properties::follow_action0)) {
		_follow_right->set_text (follow_action_to_string (trigger->follow_action (0)));
	}

	if (pc.contains (Properties::follow_action1)) {
		_follow_left->set_text (follow_action_to_string (trigger->follow_action (1)));
	}
}

/* ------------ */

TriggerWidget::TriggerWidget ()
{
	ui = new TriggerUI ();
	pack_start(*ui);
	ui->show();
//	set_background_color (UIConfiguration::instance().color (X_("theme:bg")));
}

/* ------------ */

TriggerWindow::TriggerWindow (Trigger* slot)
{
	set_title (string_compose (_("Trigger: %1"), slot->name()));

	SlotPropertiesBox* slot_prop_box = manage (new SlotPropertiesBox ());
	slot_prop_box->set_slot(slot);


	Gtk::Table* table = manage (new Gtk::Table);
	table->set_homogeneous (false);
	table->set_spacings (16);
	table->set_border_width (8);

	int col = 0;
	table->attach(*slot_prop_box,  col, col+1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND );  col++;

	if (slot->region()) {
		if (slot->region()->data_type() == DataType::AUDIO) {
			_prop_box = manage(new AudioRegionPropertiesBox ());
			_ops_box = manage(new AudioRegionOperationsBox ());
			_trim_box = manage(new AudioRegionTrimmerBox ());
		} else {
			_prop_box = manage(new MidiRegionPropertiesBox ());
			_ops_box = manage(new MidiRegionOperationsBox ());
			_trim_box = manage(new MidiRegionTrimmerBox ());
		}
		
		_prop_box->set_region(slot->region());
		_trim_box->set_region(slot->region());
		_ops_box->set_session(&slot->region()->session());

		table->attach(*_prop_box,  col, col+1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND );  col++;
		table->attach(*_trim_box,  col, col+1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND );  col++;
		table->attach(*_ops_box,   col, col+1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND );  col++;
	}

	add (*table);
	table->show_all();
}

bool
TriggerWindow::on_key_press_event (GdkEventKey* ev)
{
	Gtk::Window& main_window (ARDOUR_UI::instance()->main_window());
	return ARDOUR_UI_UTILS::relay_key_press (ev, &main_window);
}

bool
TriggerWindow::on_key_release_event (GdkEventKey* ev)
{
	Gtk::Window& main_window (ARDOUR_UI::instance()->main_window());
	return ARDOUR_UI_UTILS::relay_key_press (ev, &main_window);
}
