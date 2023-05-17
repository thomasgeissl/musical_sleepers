s = Server.default;

s.options.numAnalogInChannels = 8;
s.options.numAnalogOutChannels = 8;
s.options.numDigitalChannels = 16;
s.options.maxLogins = 4;
s.options.bindAddress = "0.0.0.0"; // allow anyone on the network connect to this server
s.options.blockSize = 16;
s.options.numInputBusChannels = 8;
s.options.numOutputBusChannels = 2;
s.options.headphoneLevel = -18;
s.options.postln;

s.waitForBoot{
	SynthDef("help-In", {
		arg out=0, in=0;
    	var input;
    	var proc;
        input = SoundIn.ar(in, 1);
        proc = PitchShift.ar(input, 0.02, Line.kr(0.1,4,20), 0, 0.0001);
        Out.ar(out, input);
	}).add;
	
	SynthDef("pulse", {
		arg out=0, out1=1, fund = 40, maxPartials = 4, width=0.5;
		var freq1, freq2, amp1, amp2, sig1, sig2, ampHz;
		ampHz = 4;
		amp1 = LFPulse.kr(ampHz, 0, 0.12) * 0.75;
		amp2 = LFPulse.kr(ampHz, 0.5, 0.12) * 0.75;

		freq1 = LFNoise0.kr(4).exprange(fund, fund*maxPartials).round(fund);
		freq2 = LFNoise0.kr(4).exprange(fund, fund*maxPartials).round(fund);
		freq1 = freq1 * LFPulse.kr(8, add: 1);
		freq2 = freq2 * LFPulse.kr(6, add: 1);

		sig1 = Pulse.ar(freq1, width, amp1);
		sig2 = Pulse.ar(freq2, width, amp1);
		
		Out.ar(out, sig1);
		Out.ar(out1, sig2);
	}).add;
	
	Synth.new("pulse",[\out, 0, \out1, 1] );
	
	SynthDef("onsetDetector", {
		arg in=0;
	    var sig = SoundIn.ar(in, 1);
		var chain = FFT(LocalBuf(512), sig);
		var threshes = (0.1, 0.2 .. 1);
	    var onsets = Onsets.kr(chain, threshes, \rcomplex);
	    // TODO: trigger samples on detection
	}).add;
	
	Synth.new("onsetDetector",[\in, 0] );



	// test all the inputs
	
	//read the input and play it out on the left channel
	Synth("help-In", [\out, 0, \in, 0]);
	Synth("help-In", [\out, 1, \in, 1]);
	
	Synth("help-In", [\out, 0, \in, 2]);
	Synth("help-In", [\out, 1, \in, 3]);
	
	Synth("help-In", [\out, 0, \in, 4]);
	Synth("help-In", [\out, 1, \in, 5]);
	
	Synth("help-In", [\out, 0, \in, 6]);
	Synth("help-In", [\out, 1, \in, 7]);

};

ServerQuit.add({ 0.exit }); // quit if the button is pressed
