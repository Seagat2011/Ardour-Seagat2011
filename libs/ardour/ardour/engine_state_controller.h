//
//  engine_state_controller.h
//  Tracks
//
//  Created by Grygorii Zharun on 4/30/14.
//  Copyright (c) 2014 Waves. All rights reserved.
//

#ifndef __gtk2_ardour__engine_state_controller__
#define __gtk2_ardour__engine_state_controller__

#include <vector>
#include <list>

#include "ardour/types.h"
#include "ardour/audio_backend.h"

namespace ARDOUR {

class AudioBackendInfo;
    
class EngineStateController
{
public:
    
    // public data types:
    struct PortState {
        std::string name;
        bool active;
        
        PortState ()
        : name(""),
        active(false)
        {
        }
        
        PortState (const std::string& name)
        : name(name),
        active(false)
        {
        }
        
        bool operator==(const PortState& rhs) {return rhs.name == name; }
        
    };
    
    typedef std::list<PortState> PortStateList;
    
    static EngineStateController* instance();
    
    //Interfaces
    void available_backends(std::vector<const AudioBackendInfo*>&);
    
    const std::string&  get_current_backend_name() const;
    
    const std::string&  get_current_device_name() const;
    void                change_current_device_to(const std::string&) const;
    void                enumerate_devices (std::vector<ARDOUR::AudioBackend::DeviceStatus>&) const;
    
    ARDOUR::framecnt_t  get_current_sample_rate() const;
    ARDOUR::framecnt_t  get_default_sample_rate() const;
    void                available_sample_rates_for_current_device(std::vector<float>&) const;
    
    ARDOUR::pframes_t   get_current_buffer_size() const;
    ARDOUR::pframes_t   get_default_buffer_size() const;
    void                available_buffer_sizes_for_current_device(std::vector<ARDOUR::pframes_t>&) const;

    // get the number of all enabled inputs/outputs
    uint32_t            get_available_inputs_count() const;
    uint32_t            get_available_outputs_count () const;
    
    // get all enabled physical input/output names
    void                get_physical_audio_inputs (std::vector<std::string>&);
    void                get_physical_audio_outputs (std::vector<std::string>&);
    
    // get all enabled midi input/output names
    void                get_physical_midi_inputs (std::vector<std::string>&);
    void                get_physical_midi_outputs (std::vector<std::string>&);
    
    // propagate new state to all inputs/outputs
    void                set_state_to_all_inputs(bool);
    void                set_state_to_all_outputs(bool); // does nothing in Stereo Out mode
    
    // get states of all inputs/outputs
    void                get_physical_audio_input_states(std::vector<PortState>&);
    void                get_physical_audio_output_states(std::vector<PortState>&);
    
    // set/get the state for input or output
    void                set_physical_audio_input_state(const std::string&, bool);
    void                set_physical_audio_output_state(const std::string&, bool);
    bool                get_physical_audio_input_state(const std::string&);
    bool                get_physical_audio_output_state(const std::string&);
    
    // get all enabled midi input/output states
    void                get_physical_midi_input_states (std::vector<PortState>&);
    void                get_physical_midi_output_states (std::vector<PortState>&);
    
    // set/get the state for input or output
    void                set_physical_midi_input_state(const std::string&, bool);
    void                set_physical_midi_output_state(const std::string&, bool);
    bool                get_physical_midi_input_state(const std::string&);
    bool                get_physical_midi_output_state(const std::string&);
    
    bool                is_setup_required() const {return ARDOUR::AudioEngine::instance()->setup_required (); }
    
    // set parameters inside the controller,
    // the state of engine won't change untill we make a "push" of this state to the backend
    // NOTE: Use push_state_to_backend() method to update backend with the most recent controller state
    bool        set_new_sample_rate_in_controller(framecnt_t);
    bool        set_new_buffer_size_in_controller(pframes_t);
    
    
    // push current controller state to backend
    bool        push_current_state_to_backend(bool);
    // switch engine to new backend
    bool        set_new_backend_as_current(const std::string&);
    // switch engine to new device
    bool        set_new_device_as_current(const std::string& device_name);
    // switch backend to session sample rate
    void        set_desired_sample_rate(framecnt_t);

    XMLNode&    serialize_audio_midi_settings();
    
    //DATE UPDATE SIGNALS
    /* this signal is emitted if the sample rate changes */
    PBD::Signal0<void> SampleRateChanged;
    /* this signal is emitted if the buffer size changes */
    PBD::Signal0<void> BufferSizeChanged;
    /* this signal is emitted if the device list changes */
    PBD::Signal1<void, bool> DeviceListChanged;
    
    //ENGINE STATE SIGNALS
    /* this signal is emitted when the engine is started */
    PBD::Signal0<void> EngineRunning;
    /* this signal is emitted when the engine is stopped */
    PBD::Signal0<void> EngineStopped;
    /* this signal is emitted if the backend ever disconnects us */
    PBD::Signal0<void> EngineHalted;
    
    /* this signals are emitted if the AUDIO i/o channel configuration changes */
    PBD::Signal0<void> InputConfigChanged;
    PBD::Signal0<void> OutputConfigChanged;
    
    /* this signals are emitted if the MIDI i/o channel configuration changes */
    PBD::Signal0<void> MIDIInputConfigChanged;
    PBD::Signal0<void> MIDIOutputConfigChanged;
    
    /* this signal is emitted if new ports are registered or unregistered */
    PBD::Signal0<void> PortRegistrationChanged;
    
private:

    EngineStateController(); // singleton
    ~EngineStateController(); // singleton
    EngineStateController(const EngineStateController& ); // prohibited
    EngineStateController& operator=(const EngineStateController&); // prohibited
    
    // private data structures
    
    // Engine state
    struct State {
		std::string backend_name;
		std::string device_name;
		ARDOUR::framecnt_t sample_rate;
		ARDOUR::pframes_t buffer_size;
		//uint32_t input_latency; not used so far
		//uint32_t output_latency; not used so far
		PortStateList input_channel_states;
		PortStateList multi_out_channel_states;
        PortStateList stereo_out_channel_states;
		bool active;
		//std::string midi_option; not used so far
        
		State()
        : sample_rate(0)
        , buffer_size(0)
        , input_channel_states (0)
        , multi_out_channel_states (0)
        , stereo_out_channel_states (0)
        , active (false)
        {
        }
        
        bool operator==(const State& rhs)
        {
            return (backend_name == rhs.backend_name) && (device_name == rhs.device_name);
        }
        
        std::string form_state_name() {
            return std::string("State:" + backend_name + ":" + device_name);
        }
        
        // predicates for search
        struct StatePredicate
        {
            StatePredicate(const std::string& backend_name, const std::string& device_name)
            : _backend_name (backend_name)
            , _device_name (device_name)
            {}
            
            bool operator()(boost::shared_ptr<ARDOUR::EngineStateController::State> rhs)
            {
                return (_backend_name == rhs->backend_name) && (_device_name == rhs->device_name);
            }
            
        private:
            std::string _backend_name;
            std::string _device_name;
        };
    };
    
    typedef boost::shared_ptr<State> StatePtr;
    typedef std::list<StatePtr> StateList;
    
    // MIDI device data structures
    
    struct MidiPortState
    {
        std::string name;
        bool active;
        bool available;
        
        MidiPortState(const std::string& name):
        name(name),
        active(false),
        available(false)
        {}
        
        bool operator==(const MidiPortState& rhs)
        {
            return name == rhs.name;
        }
    };
    
    typedef std::list<MidiPortState> MidiPortStateList;
    
    // state control methods////////////////
    void _deserialize_and_load_engine_states();
    void _deserialize_and_load_midi_port_states() {};
    void _serialize_engine_states(XMLNode*);
    void _serialize_midi_port_states(XMLNode*) {};
    // sets last active state as current state
    // if no last active state found it loads default state
    void _do_initial_engine_setup();
    bool _apply_state(const StatePtr& state);
    // get gets available device channels from engine and updates internal controller state
    void _update_device_channels_state(bool reconnect_session_routes = true);
    // check stereo out channel state configuration and make it correcpond stereo out mode requirements
    void _refresh_stereo_out_channel_states();
    ////////////////////////////////////////
    
    // internal helper functions////////////
    // make sure that current device parameters are supported and fit session requirements 
    void _validate_current_device_state();
    ////////////////////////////////////////
        
    ////////////////////////////////////////
    // callbacks
    void _on_engine_running();
    void _on_engine_halted();
    void _on_engine_stopped();
    void _on_sample_rate_change(ARDOUR::framecnt_t);
    void _on_buffer_size_change(ARDOUR::pframes_t);
    void _on_device_list_change();
    void _on_parameter_changed (const std::string&);
    void _on_ports_registration_update ();
    ////////////////////////////////////////
        
    ////////////////////////////////////////
    // attributes
    StatePtr _current_state;
    // list of system states
    StateList _states;
    
    MidiPortStateList _midi_inputs;
    MidiPortStateList _midi_outputs;
    
    // last active non-default (real) device
    std::string _last_used_real_device;
    
    ARDOUR::framecnt_t  _desired_sample_rate;
    bool                _have_control;
    
    // Engine connections stuff
	PBD::ScopedConnectionList update_connections;
    PBD::ScopedConnection running_connection;
    PBD::ScopedConnection halt_connection;
    PBD::ScopedConnection stopped_connection;

};

} // namespace ARDOUR
    
#endif /* defined(__gtk2_ardour__engine_state_controller__) */
