﻿#include <map>

#include "page/diagnostics/hydraulic_pump_dx.hpp"
#include "configuration.hpp"

#include "datum/string.hpp"

#include "graphlet/shapelet.hpp"
#include "graphlet/dashboard/alarmlet.hpp"

#include "iotables/di_pumps.hpp"
#include "iotables/di_valves.hpp"
#include "iotables/di_devices.hpp"
#include "iotables/di_dredges.hpp"
#include "iotables/di_hopper_pumps.hpp"

#include "module.hpp"
#include "brushes.hxx"

using namespace WarGrey::SCADA;
using namespace WarGrey::GYDM;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static CanvasSolidColorBrush^ region_background = Colours::make(diagnostics_region_background);
static CanvasSolidColorBrush^ diagnosis_background = Colours::make(diagnostics_alarm_background);

static CanvasSolidColorBrush^ subcolor = Colours::DimGray;
static CanvasSolidColorBrush^ subcolor_highlight = Colours::DodgerBlue;
static CanvasSolidColorBrush^ caption_color = Colours::Salmon;
static CanvasSolidColorBrush^ diagnosis_color = Colours::Silver;

// WARNING: order matters
private enum class P : unsigned int {
	// Groups
	MiscCondition, PumpCondition,

	// Addition Labels
	//SQk1Open, SQk2Open,

	// Misc Conditions
	NoConsolePSStop, NoConsoleSBStop, NoSailingStop, NoPSWinchGantryStop, NoSBWinchGantryStop,
	NoMasterTankFS01, NoMasterTankFS02, NoMasterTankLS1, NoMasterTankLS2,
	NoVisorTankFS, NoVisorTankLS1, NoVisorTankLS2,
	MasterOilTemperaturesNormal,//主油温正常
	VisorOilTemperaturesNormal,//耙唇油箱油温正常
	CoolantRunning,// Flushing,

	// Pump Conditions
	Remote, NoRunning, NoBroken,

	/*
	SQaOpen, SQbOpen, SQgOpen, SQhOpen,
	SQcOpen, SQfOpen, SQdOpen, SQeOpen,
	SQiOpen, SQjOpen,
	SQyOpen, SQlOpen, SQmOpen, SQkOpen,

	NoC2A, NoC2B, NoF2G, NoF2H,
	NoA2C, NoB2C, NoF2C, NoC2F, NoH2F, NoG2F,
	NoJ2I, NoI2J,*/

	S4Open, S5Open, S14Open, S13Open, S12Open, S11Open, S1Open,
	S7Open, S24Open, S23Open, S22Open, S21Open, S2Open,

	NoA1ToA2, NoA2ToA1, NoB12ToB34, NoB34ToB12, NoC1ToC2, NoC2ToC1, NoJToI, NoIToJ,

	_
};

static P other[] = { P::MasterOilTemperaturesNormal, P::NoMasterTankFS02, P::NoSailingStop, P::NoMasterTankLS1, P::NoMasterTankLS2 };
static P visor[] = { P::VisorOilTemperaturesNormal, P::NoSailingStop, P::NoVisorTankFS, P::NoVisorTankLS1, P::NoVisorTankLS2 };
static P ps[] = { P::MasterOilTemperaturesNormal,P::NoConsolePSStop, P::NoSailingStop, P::NoPSWinchGantryStop, P::NoMasterTankFS01,
	P::NoMasterTankLS1, P::NoMasterTankLS2, P::S1Open, P::CoolantRunning };
static P sb[] = { P::MasterOilTemperaturesNormal, P::NoConsoleSBStop, P::NoSailingStop, P::NoSBWinchGantryStop, P::NoMasterTankFS01,
	P::NoMasterTankLS1, P::NoMasterTankLS2, P::S2Open, P::CoolantRunning };

/*
static P A[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQaOpen, P::NoC2A };
static P B[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQbOpen, P::NoC2B };
static P G[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQgOpen, P::NoF2G };
static P H[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQhOpen, P::NoF2H };

static P C[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQcOpen, P::NoA2C, P::NoB2C, P::NoF2C };
static P F[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQfOpen, P::NoC2F, P::NoH2F, P::NoG2F };
static P D[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQdOpen };
static P E[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQeOpen };

static P I[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQyOpen, P::NoJ2I };
static P J[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQjOpen, P::NoI2J };

static P Y[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQyOpen };
static P L[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQlOpen };
static P M[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQmOpen };
static P K[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQkOpen };
*/
static P D1[] = { P::Remote, P::NoRunning, P::NoBroken, P::S4Open };
static P D2[] = { P::Remote, P::NoRunning, P::NoBroken, P::S5Open };
static P C1[] = { P::Remote, P::NoRunning, P::NoBroken, P::S14Open, P::NoC2ToC1 };
static P B2[] = { P::Remote, P::NoRunning, P::NoBroken, P::S13Open, P::NoB34ToB12 };
static P B1[] = { P::Remote, P::NoRunning, P::NoBroken, P::S12Open, P::NoB34ToB12 };
static P A1[] = { P::Remote, P::NoRunning, P::NoBroken, P::S11Open, P::NoA2ToA1 };

static P F1[] = { P::Remote, P::NoRunning, P::NoBroken, P::S7Open };
static P F2[] = { P::Remote, P::NoRunning, P::NoBroken, P::S7Open };
static P C2[] = { P::Remote, P::NoRunning, P::NoBroken, P::S14Open, P::NoC1ToC2 };
static P B4[] = { P::Remote, P::NoRunning, P::NoBroken, P::S13Open, P::NoB12ToB34 };
static P B3[] = { P::Remote, P::NoRunning, P::NoBroken, P::S12Open, P::NoB12ToB34 };
static P A2[] = { P::Remote, P::NoRunning, P::NoBroken, P::S11Open, P::NoA1ToA2 };
static P E[] = { P::Remote, P::NoRunning, P::NoBroken };

static P J[] = { P::Remote, P::NoRunning, P::NoBroken ,P::NoIToJ };
static P I[] = { P::Remote, P::NoRunning, P::NoBroken ,P::NoJToI };


static const P* select_group_conditions(HPDX gid, unsigned int* count) {
	const P* group = nullptr;

	switch (gid) {
	case HPDX::PS: group = ps; SET_BOX(count, sizeof(ps) / sizeof(P)); break;
	case HPDX::SB: group = sb; SET_BOX(count, sizeof(sb) / sizeof(P)); break;
	case HPDX::Visor: group = visor; SET_BOX(count, sizeof(visor) / sizeof(P)); break;
	case HPDX::Other: group = other; SET_BOX(count, sizeof(other) / sizeof(P)); break;
	}

	return group;
}

static const P* select_pump_conditions(unsigned int id, unsigned int* count) {
	const P* pump = nullptr;

	switch (id) {
	case pump_D1_feedback: pump = D1; SET_BOX(count, sizeof(D1) / sizeof(P)); break;
	case pump_D2_feedback: pump = D2; SET_BOX(count, sizeof(D2) / sizeof(P)); break;
	case pump_C1_feedback: pump = C1; SET_BOX(count, sizeof(C1) / sizeof(P)); break;
	case pump_B2_feedback: pump = B2; SET_BOX(count, sizeof(B2) / sizeof(P)); break;
	case pump_B1_feedback: pump = B1; SET_BOX(count, sizeof(B1) / sizeof(P)); break;
	case pump_A1_feedback: pump = A1; SET_BOX(count, sizeof(A1) / sizeof(P)); break;
	case pump_F1_feedback: pump = F1; SET_BOX(count, sizeof(F1) / sizeof(P)); break;
	case pump_F2_feedback: pump = F2; SET_BOX(count, sizeof(F2) / sizeof(P)); break;
	case pump_C2_feedback: pump = C2; SET_BOX(count, sizeof(C2) / sizeof(P)); break;
	case pump_B4_feedback: pump = B4; SET_BOX(count, sizeof(B4) / sizeof(P)); break;
	case pump_B3_feedback: pump = B3; SET_BOX(count, sizeof(B3) / sizeof(P)); break;
	case pump_A2_feedback: pump = A2; SET_BOX(count, sizeof(A2) / sizeof(P)); break;
	case pump_E_feedback: pump = E; SET_BOX(count, sizeof(E) / sizeof(P)); break;
	case pump_I_feedback: pump = I; SET_BOX(count, sizeof(I) / sizeof(P)); break;
	case pump_J_feedback: pump = J; SET_BOX(count, sizeof(J) / sizeof(P)); break;
	}

	return pump;
}

/*************************************************************************************************/
private class PumpDx final : public PLCConfirmation {
public:
	PumpDx(HydraulicPumpDiagnostics* master) : master(master) {
		this->region_font = make_bold_text_format("Microsoft YaHei", normal_font_size);
		this->diagnosis_font = make_bold_text_format("Microsoft YaHei", small_font_size);
		this->subfont = make_bold_text_format("Microsoft YaHei", tiny_font_size);
	}

public:
	void pre_read_data(Syslog* logger) override {
		this->master->enter_critical_section();
		this->master->begin_update_sequence();
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		this->diagnoses[P::MasterOilTemperaturesNormal]->set_state(
			(DBX(DB4, master_tank_temperature_high_alarm - 1U)) || (DBX(DB4, master_tank_temperature_low_alarm - 1U))
			, AlarmState::None, AlarmState::Notice);//已核对
		this->diagnoses[P::VisorOilTemperaturesNormal]->set_state(
			(!DBX(DB4, visor_tank_temperature_high_alarm - 1U)) && (!DBX(DB4, visor_tank_temperature_hlow_alarm - 1U))
			, AlarmState::None, AlarmState::Notice);//已核对
		this->diagnoses[P::NoConsolePSStop]->set_state(DBX(DB4, console_ps_hydraulics_stop_button - 1U), AlarmState::None, AlarmState::Notice);//已核对
		this->diagnoses[P::NoConsoleSBStop]->set_state(DBX(DB4, console_sb_hydraulics_stop_button - 1U), AlarmState::None, AlarmState::Notice);//已核对
		this->diagnoses[P::NoSailingStop]->set_state(DBX(DB4, sailing_hydraulics_stop_button - 1U), AlarmState::None, AlarmState::Notice);//已核对
		this->diagnoses[P::NoPSWinchGantryStop]->set_state(DBX(DB4, console_ps_winch_gantry_stop_button - 1U), AlarmState::None, AlarmState::Notice);//已核对
		this->diagnoses[P::NoSBWinchGantryStop]->set_state(DBX(DB4, console_sb_winch_gantry_stop_button - 1U), AlarmState::None, AlarmState::Notice);//已核对
		//this->diagnoses[P::NoMasterTankFS01]->set_state(DBX(DB4, filter_01_status - 1U), AlarmState::None, AlarmState::Notice);
		//this->diagnoses[P::NoMasterTankFS02]->set_state(DBX(DB4, filter_02_status - 1U), AlarmState::None, AlarmState::Notice);
		//this->diagnoses[P::NoVisorTankFS]->set_state(DBX(DB4, filter_10_status - 1U), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoMasterTankLS1]->set_state(DI_tank_level_low(DB4, master_tank_status), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoMasterTankLS2]->set_state(DI_tank_level_too_low(DB4, master_tank_status), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoVisorTankLS1]->set_state(DBX(DB4, visor_tank_status - 1U), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoVisorTankLS2]->set_state(DBX(DB4, visor_tank_status), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::S1Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S1_status), AlarmState::Notice, AlarmState::None);
		this->diagnoses[P::S2Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S2_status), AlarmState::Notice, AlarmState::None);
		this->diagnoses[P::CoolantRunning]->set_state(DI_hydraulic_pump_running(DB4, pump_F1_feedback)|| DI_hydraulic_pump_running(DB4, pump_F2_feedback), AlarmState::Notice, AlarmState::None);//循环冷却泵
		//this->diagnoses[P::Flushing]->set_state(DI_hydraulic_pump_running(DB4, pump_L_feedback), AlarmState::Notice, AlarmState::None);

		{ // check pumps
			unsigned int feedback = this->master->get_id();

			this->diagnoses[P::Remote]->set_state(DI_hydraulic_pump_remote_control(DB4, this->feedback), AlarmState::Notice, AlarmState::None);
			this->diagnoses[P::NoRunning]->set_state(DI_hydraulic_pump_running(DB4, feedback), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoBroken]->set_state(DI_hydraulic_pump_broken(DB4, feedback), AlarmState::None, AlarmState::Notice);


			this->diagnoses[P::NoC1ToC2]->set_state(DBX(DB4, pump_C1_replace_C2 - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoC2ToC1]->set_state(DBX(DB4, pump_C2_replace_C1 - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoB12ToB34]->set_state(DBX(DB4, pump_B12_replace_B34 - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoB34ToB12]->set_state(DBX(DB4, pump_B34_replace_B12 - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoA1ToA2]->set_state(DBX(DB4, pump_A1_replace_A2 - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoA2ToA1]->set_state(DBX(DB4, pump_A2_replace_A1 - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoJToI]->set_state(DBX(DB4, pump_J_replace_I - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoIToJ]->set_state(DBX(DB4, pump_I_replace_J - 1U), AlarmState::None, AlarmState::Notice);

			{ // check valves
				//bool k1_open = DI_manual_valve_open(DB4, manual_valve_SQk1_status);
				//bool k2_open = DI_manual_valve_open(DB4, manual_valve_SQk2_status);

				this->diagnoses[P::S4Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S4_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::S5Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S5_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::S14Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S14_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::S13Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S13_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::S12Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S12_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::S11Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S11_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::S1Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S1_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::S7Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S7_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::S24Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S24_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::S23Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S23_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::S22Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S22_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::S21Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S21_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::S2Open]->set_state(DI_manual_valve_open(DB4, manual_valve_S2_status), AlarmState::Notice, AlarmState::None);

				//this->diagnoses[P::SQkOpen]->set_state((k1_open || k2_open), AlarmState::Notice, AlarmState::None);

				//this->labels[P::SQk1Open]->set_color(k1_open ? subcolor_highlight : subcolor);
				//this->labels[P::SQk2Open]->set_color(k2_open ? subcolor_highlight : subcolor);
			}
		}
	}

	void post_read_data(Syslog* logger) override {
		this->master->end_update_sequence();
		this->master->leave_critical_section();
	}

public:
	void fill_extent(float title_height, float vgapsize, float* width, float* height) {//!!如果这里屏蔽掉会导致排版错误
		
		unsigned int gc_count, pc_count;
		float region_reserved_height = vgapsize * 4.0F + this->region_font->FontSize;

		select_group_conditions(HPDX::PS, &gc_count);
		select_pump_conditions(pump_B3_feedback, &pc_count);//!!!!这是随便写的

		this->diagnosis_height = this->diagnosis_font->FontSize * 2.0F;
		this->misc_region_height = (this->diagnosis_height + vgapsize) * float(gc_count) + region_reserved_height;
		this->pump_region_height = (this->diagnosis_height + vgapsize) * float(pc_count) + region_reserved_height;

		SET_BOX(width, 400.0F);
		SET_BOX(height, this->misc_region_height + this->pump_region_height + title_height * 3.0F);
	}

	void load(float width, float height, float title_height, float vgapsize) {
		float region_width = width * 0.90F;
		float diagnosis_width = (region_width - title_height * 1.5F);
		float corner_radius = 8.0F;

		this->misc_region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->misc_region_height, corner_radius, region_background));

		this->pump_region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->pump_region_height, corner_radius, region_background));

		this->load_label(this->labels, P::MiscCondition, P::MiscCondition.ToString(), caption_color, this->region_font);
		this->load_label(this->labels, P::PumpCondition, P::PumpCondition.ToString(), caption_color, this->region_font);

		{ // load diagnoses
			float icon_size = this->diagnosis_height * 0.618F;

			for (P id = P::NoConsolePSStop; id < P::_; id++) {
				this->slots[id] = this->master->insert_one(new Credit<RoundedRectanglet, P>(
					diagnosis_width, this->diagnosis_height, corner_radius, diagnosis_background), id);

				this->diagnoses[id] = this->master->insert_one(new Credit<Alarmlet, P>(icon_size), id);
				this->load_label(this->labels, id, diagnosis_color, this->diagnosis_font);
			}

			//this->load_label(this->labels, P::SQk1Open, subcolor, this->subfont);
			//this->load_label(this->labels, P::SQk2Open, subcolor, this->subfont);
		}
	}

	void reflow(float width, float height, float title_height, float vgapsize) {
		unsigned int gc_count = 0;
		unsigned int pc_count = 0;
		unsigned int feedback = this->master->get_id();
		const P* group = select_group_conditions(this->group, &gc_count);
		const P* pump = select_pump_conditions(feedback, &pc_count);

		{ // reflow layout
			float gapsize = (height - title_height - this->misc_region_height - this->pump_region_height) / 3.0F;

			this->master->move_to(this->misc_region, width * 0.5F, title_height + gapsize, GraphletAnchor::CT);
			this->master->move_to(this->pump_region, this->misc_region, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, gapsize);

			this->master->move_to(this->labels[P::MiscCondition], this->misc_region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
			this->master->move_to(this->labels[P::PumpCondition], this->pump_region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
		}

		{ // conceal irrelevant diagnosis
			for (P id = P::NoConsolePSStop; id < P::_; id++) {
				this->master->move_to(this->slots[id], 0.0F, 0.0F);
			}
		}

		this->reflow(this->slots, P::MiscCondition, group, gc_count, GraphletAnchor::CB, GraphletAnchor::CT, vgapsize);
		this->reflow(this->slots, P::PumpCondition, pump, pc_count, GraphletAnchor::CB, GraphletAnchor::CT, vgapsize);

		{ // reflow diagnostics
			float inset = vgapsize * 1.618F;
			float step = vgapsize;
			float icon_width;

			this->diagnoses[P::NoConsolePSStop]->fill_extent(0.0F, 0.0F, &icon_width, nullptr);
			step += icon_width;

			for (P id = P::NoConsolePSStop; id < P::_; id++) {
				this->master->move_to(this->diagnoses[id], this->slots[id], GraphletAnchor::LC, GraphletAnchor::LC, step * 0.0F + inset);
				this->master->move_to(this->labels[id], this->slots[id], GraphletAnchor::LC, GraphletAnchor::LC, step * 1.0F + inset + vgapsize);
			}
			/*
			if (feedback == pump_K_feedback) {
				this->master->move_to(this->labels[P::SQk1Open], this->labels[P::SQkOpen], GraphletAnchor::RB, GraphletAnchor::LB, vgapsize);
				this->master->move_to(this->labels[P::SQk2Open], this->labels[P::SQk1Open], GraphletAnchor::RB, GraphletAnchor::LB, vgapsize);
			} else {
				this->master->move_to(this->labels[P::SQk1Open], 0.0F, 0.0F);
				this->master->move_to(this->labels[P::SQk2Open], 0.0F, 0.0F);
			}*/
		}
	}

public:
	bool available() override {
		return (this->master->surface_ready() && this->master->shown());
	}

	void set_pump(Platform::String^ name, HPDX gid, unsigned int feedback, unsigned int details) {
		this->details = details;
		this->feedback = feedback;

		// delay the updating when `switch_id`
		this->pump = name;
		this->group = gid;
	}

	void switch_id(unsigned int id) {
		// `ICreditSatellite` guarantees that the master is loaded and reflowed;
		// furthermore, the `this->master->get_id()` works for `load` and `reflow` even when `switch_id` is delayed.
		this->labels[P::MiscCondition]->set_text(_speak(this->group.ToString() + P::MiscCondition.ToString()), GraphletAnchor::CC);
		this->labels[P::PumpCondition]->set_text(_speak(this->pump + P::PumpCondition.ToString()), GraphletAnchor::CC);
	}

private:
	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, E id, CanvasSolidColorBrush^ color, CanvasTextFormat^ font = nullptr) {
		ls[id] = this->master->insert_one(new Credit<Labellet, E>(_speak(id), font, color), id);
	}

	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, E id, Platform::String^ prefix, CanvasSolidColorBrush^ color, CanvasTextFormat^ font = nullptr) {
		ls[id] = this->master->insert_one(new Credit<Labellet, E>(_speak(prefix + id.ToString()), font, color), id);
	}

private:
	template<class G, typename E>
	void reflow(std::map<E, Credit<G, E>*>& gs, E label, const E* ds, unsigned int count, GraphletAnchor ta, GraphletAnchor a, float yoff) {
		IGraphlet* target = this->labels[label];

		for (unsigned int idx = 0; idx < count; idx++) {
			auto g = gs[ds[idx]];

			this->master->move_to(g, target, ta, a, 0.0F, yoff);
			target = g;
		}
	}

private: // never delete these graphlets mannually
	std::map<P, Credit<Labellet, P>*> labels;
	std::map<P, Credit<Alarmlet, P>*> diagnoses;
	std::map<P, Credit<RoundedRectanglet, P>*> slots;
	RoundedRectanglet* misc_region;
	RoundedRectanglet* pump_region;

private:
	CanvasTextFormat^ region_font;
	CanvasTextFormat^ diagnosis_font;
	CanvasTextFormat^ subfont;

private:
	float diagnosis_height;
	float misc_region_height;
	float pump_region_height;

private:
	HydraulicPumpDiagnostics* master;
	Platform::String^ pump;
	unsigned int details;
	unsigned int feedback;
	HPDX group;
};

HydraulicPumpDiagnostics::HydraulicPumpDiagnostics(PLCMaster* plc) : ICreditSatellite(plc->get_logger(), __MODULE__), device(plc) {
	PumpDx* dashboard = new PumpDx(this);

	this->dashboard = dashboard;

	this->device->push_confirmation_receiver(dashboard);
}

HydraulicPumpDiagnostics::~HydraulicPumpDiagnostics() {
	if (this->dashboard != nullptr) {
		delete this->dashboard;
	}
}

void HydraulicPumpDiagnostics::fill_extent(float* width, float* height) {
	auto dashboard = dynamic_cast<PumpDx*>(this->dashboard);
	float db_width = 400.0F;
	float db_height = 600.0F;

	this->title_height = large_font_size * 2.0F;
	this->vgapsize = this->title_height * 0.16F;

	if (dashboard != nullptr) {
		dashboard->fill_extent(this->title_height, this->vgapsize, &db_width, &db_height);
	}

	SET_BOX(width, db_width);
	SET_BOX(height, db_height);
}

void HydraulicPumpDiagnostics::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto dashboard = dynamic_cast<PumpDx*>(this->dashboard);

	if (dashboard != nullptr) {
		auto caption_font = make_bold_text_format("Microsoft YaHei", large_font_size);

		dashboard->load(width, height, this->title_height, this->vgapsize);

		this->titlebar = this->insert_one(new Rectanglet(width, this->title_height, Colours::make(diagnostics_caption_background)));
		this->title = this->insert_one(new Labellet(this->display_name(), caption_font, diagnostics_caption_foreground));
	}
}

void HydraulicPumpDiagnostics::reflow(float width, float height) {
	auto dashboard = dynamic_cast<PumpDx*>(this->dashboard);

	if (dashboard != nullptr) {
		dashboard->reflow(width, height, this->title_height, this->vgapsize);
		this->move_to(this->title, this->titlebar, GraphletAnchor::CC, GraphletAnchor::CC);
	}
}

void HydraulicPumpDiagnostics::set_pump(Platform::String^ id, HPDX group, unsigned int feedback , unsigned int details) {
	auto dashboard = dynamic_cast<PumpDx*>(this->dashboard);

	if (dashboard != nullptr) {
		dashboard->set_pump(id, group, feedback, details);
	}
}

void HydraulicPumpDiagnostics::on_id_changed(unsigned int id) {
	auto dashboard = dynamic_cast<PumpDx*>(this->dashboard);

	if (dashboard != nullptr) {
		dashboard->switch_id(id);
	}
}
