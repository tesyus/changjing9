#pragma once

#include "timemachine.hpp"
#include "planet.hpp"
#include "plc.hpp"

#include "decorator/grid.hpp"
#include <page\diagnostics\hopper_pump_dx.hpp>

namespace WarGrey::SCADA {
	private class DischargesPage : public WarGrey::SCADA::Planet, public WarGrey::SCADA::ITimeMachineListener {
	public:
		virtual ~DischargesPage() noexcept;
		DischargesPage(WarGrey::SCADA::PLCMaster* plc = nullptr);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;

	public:
		void on_timestream(long long time_ms, size_t addr0, size_t addrn, uint8* data, size_t size, uint64 p_type, size_t p_size, WarGrey::GYDM::Syslog* logger) override;

	public:
		bool can_select(IGraphlet* g) override;
		bool can_select_multiple() override;
		void on_tap_selected(IGraphlet* g, float x, float y) override;
		void on_gesture(std::list<Windows::Foundation::Numerics::float2>& anchors, float x, float y) override;
		void on_focus(IGraphlet* g, bool yes_no) override;
		bool on_key(Windows::System::VirtualKey key, bool wargrey_keyboard) override;

	private:
		WarGrey::SCADA::PLCMaster* device;
		WarGrey::SCADA::PLCConfirmation* dashboard;
		WarGrey::SCADA::ISatellite* diagnostics;
		Windows::UI::Xaml::Controls::MenuFlyout^ anchor_winch_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ barge_winch_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ barge_cylinder_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ shore_winch_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ shore_cylinder_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ gate_valve_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ upper_door_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ ps_hopper_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ sb_hopper_op;
		Windows::UI::Xaml::Controls::MenuFlyout^ gdischarge_op;

		Windows::UI::Xaml::Controls::MenuFlyout^ motor_valve_op;

	private:
		WarGrey::SCADA::GridDecorator* grid;
	};
}
