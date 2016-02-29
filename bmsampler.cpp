
#include <string>
#include <iostream>
#include <list>
#include <sndfile.hh>
#include "include/json.hpp"
#define MAX_FRAMES (44100 * 60 * 15)

using json = nlohmann::json;
using namespace std;

struct Frame {
	float data[2];
};

class Sound {
	vector<float> data;
public:
	int frames;
	Sound (string path) {
		SndfileHandle sf = SndfileHandle(path);
		if (sf.channels() != 2) {
			throw "Sample must be strero!";
		}
		if (sf.samplerate() != 44100) {
			throw "Sample must be 44100hz";
		}
		frames = sf.frames();
		data.resize(frames * 2);
		sf.readf(data.data(), frames);
	}
	Frame read (int frame_number) {
		if (is_done(frame_number)) return { { 0, 0 } };
		int i = frame_number * 2;
		return { { data[i], data[i + 1] } };
	}
	bool is_done (int frame_number) {
		return frame_number >= frames;
	}
};

class Sampler {
	shared_ptr<Sound> sound_ptr;
	int current_frame;
public:
	string key;
	Sampler (string key, shared_ptr<Sound> sound_ptr) {
		this->sound_ptr = sound_ptr;
		this->key = key;
	}
	shared_ptr<Sound> sound() {
		return sound_ptr;
	}
	void reset () {
		current_frame = 0;
	}
	Frame read () {
		return sound_ptr->read(current_frame++);
	}
	bool is_done () {
		return current_frame >= this->sound_ptr->frames;
	}
};

class SamplerPool {
	map<string, shared_ptr<Sampler>> active_samplers;
public:
	void add (shared_ptr<Sampler> sampler) {
		active_samplers[sampler->key] = sampler;
	}
	Frame read () {
		Frame out = { { 0, 0 } };
		vector<string> done_keys;
		for (auto entry : active_samplers) {
			Frame read = entry.second->read();
			out.data[0] += read.data[0];
			out.data[1] += read.data[1];
			if (entry.second->is_done()) done_keys.push_back(entry.first);
		}
		for (auto key : done_keys) active_samplers.erase(key);
		return out;
	}
};

struct Event {
	string key;
	int frame;
};

bool compare_events(const Event &e1, const Event &e2) {
	return e1.frame < e2.frame;
}

class BMSampler {
	map<string, shared_ptr<Sampler> > sampler_map;
	map<string, shared_ptr<Sound> > sound_map;
	vector<Event> events;
	int song_length = 0;
public:
	void sample(string key, string path) {
		if (sound_map.find(path) == sound_map.end()) {
			shared_ptr<Sound> sound_ptr(new Sound(path));
			sound_map[path] = sound_ptr;
		}
		shared_ptr<Sampler> sampler_ptr(new Sampler(key, sound_map[path]));
		sampler_map[key] = sampler_ptr;
		clog << " " << key << (sampler_map.size() % 26 == 0 ? "\n" : "");
	}
	void play(string key, double time) {
		Event event { key, int(time * 44100) };
		events.push_back(event);
		auto sample = sampler_map[key]->sound();
		song_length = max(song_length, event.frame + sample->frames);
	}
	void go(string path) {
		sort(events.begin(), events.end(), compare_events);
		int length = min(song_length, MAX_FRAMES);
		int samples = length * 2;
		int printed = 0;
		auto output = vector<float>(samples * 2);
		auto event_iterator = events.begin();
		SamplerPool pool;
		cerr << endl << endl << "Now rendering..." << endl;
		for (int i = 0; i < length; i ++) {
			int dots = i * 80 / length;
			while (event_iterator != events.end()) {
				if (i < event_iterator->frame) break;
				shared_ptr<Sampler> sampler = sampler_map[event_iterator->key];
				sampler->reset();
				pool.add(sampler);
				event_iterator++;
			}
			for (; printed < dots; printed ++) cerr << ".";
			Frame frame = pool.read();
			output[i * 2] = frame.data[0];
			output[i * 2 + 1] = frame.data[1];
		}
		cerr << "\n";
		clog << "Write result to " << path << endl;
		SndfileHandle wav(path, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_FLOAT, 2, 44100);
		wav.writef(output.data(), length);
	}
};

int main (int argc, char* argv[]) {
	string line;
	BMSampler bms;
	if (argc <= 1) {
		cerr << "Usage: Specify the output file!!" << endl;
		return 1;
	}
	if (string(argv[1]) == "-v") {
		cout << "v0.1.0" << endl;
		return 0;
	}
	try {
		while (getline(cin, line)) {
			json command = json::parse(line);
			string type = command["type"];
			if (type == "sample") {
				string key = command["key"];
				string path = command["path"];
				bms.sample(key, path);
			} else if (type == "play") {
				string key = command["key"];
				double time = command["time"];
				bms.play(key, time);
			} else if (type == "go") {
				bms.go(string(argv[1]));
				return 0;
			} else {
				clog << "Unrecognized command: " << type << endl;
			}
		}
		return 0;
	} catch (const char* msg) {
		cerr << "ERROR: " << msg;
		return 1;
	}
}
