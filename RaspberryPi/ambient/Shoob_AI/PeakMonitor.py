import sys
import time
from Queue import Queue
from ctypes import POINTER, c_ubyte, c_void_p, c_ulong, cast
from pulseaudio.lib_pulseaudio import *
import NeoPixel


class PeakMonitor(object):
    def __init__(self, sink_name, rate):
        print "STARTING PULSEAUDIO"
        self.sink_name = sink_name
        self.rate = rate

        self._context_notify_cb = pa_context_notify_cb_t(self.context_notify_cb)
        self._sink_info_cb = pa_sink_info_cb_t(self.sink_info_cb)
        self._stream_read_cb = pa_stream_request_cb_t(self.stream_read_cb)

        self._samples = Queue()

        _mainloop = pa_threaded_mainloop_new()
        _mainloop_api = pa_threaded_mainloop_get_api(_mainloop)
        context = pa_context_new(_mainloop_api, 'peak_demo')
        pa_context_set_state_callback(context, self._context_notify_cb, None)
        pa_context_connect(context, None, 0, None)
        pa_threaded_mainloop_start(_mainloop)

    def __iter__(self):
        while True:
            yield self._samples.get()

    def context_notify_cb(self, context, _):
        state = pa_context_get_state(context)
        if state == PA_CONTEXT_READY:
            print "Pulseaudio connection ready..."
            o = pa_context_get_sink_info_list(context, self._sink_info_cb, None)
            pa_operation_unref(o)

        elif state == PA_CONTEXT_FAILED:
            global setupDone
            setupDone = True
            NeoPixel.errorRed()
            print "Connection PulseAudio failed"
            time.sleep(3)
            sys.exit()

        elif state == PA_CONTEXT_TERMINATED:
            NeoPixel.errorRed()
            print "Connection PulseAudio terminated"
            time.sleep(3)
            sys.exit()

    def sink_info_cb(self, context, sink_info_p, _, __):
        if not sink_info_p:
            return

        sink_info = sink_info_p.contents
        print 'index:', sink_info.index
        print 'name:', sink_info.name
        print 'description:', sink_info.description

        if sink_info.name == self.sink_name:
            print 'setting up peak recording using', sink_info.monitor_source_name

            print '-' * 60
            samplespec = pa_sample_spec()
            samplespec.channels = 1
            samplespec.format = PA_SAMPLE_U8
            samplespec.rate = self.rate

            pa_stream = pa_stream_new(context, "peak detect demo", samplespec, None)
            pa_stream_set_read_callback(pa_stream,
                                        self._stream_read_cb,
                                        sink_info.index)
            pa_stream_connect_record(pa_stream,
                                     sink_info.monitor_source_name,
                                     None,
                                     PA_STREAM_PEAK_DETECT)

    def stream_read_cb(self, stream, length, index_incr):
        data = c_void_p()
        pa_stream_peek(stream, data, c_ulong(length))
        data = cast(data, POINTER(c_ubyte))
        for i in xrange(length):
            self._samples.put(data[i])
        pa_stream_drop(stream)
