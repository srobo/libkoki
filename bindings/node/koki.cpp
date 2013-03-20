#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
#include <node_buffer.h>
#include <opencv/highgui.h>
#include "koki.h"

using namespace node;
using namespace v8;

koki_t *koki;

static GPtrArray* process_jpeg(unsigned char* data, int length) {
    GPtrArray *markers;
    CvMat *m = cvCreateMatHeader (length, 1, CV_8UC1);
    cvSetData (m, data, 1);
    IplImage *color = cvDecodeImage (m, CV_LOAD_IMAGE_COLOR);
    assert (color != NULL);

    IplImage *bw = cvCreateImage(cvGetSize(color), color->depth, 1);
    cvCvtColor (color, bw, CV_RGB2GRAY);

    koki_camera_params_t params;
    params.size.x = bw->width;
    params.size.y = bw->height;
    params.principal_point.x = params.size.x / 2;
    params.principal_point.y = params.size.y / 2;
    params.focal_length.x = 571.0;
    params.focal_length.y = 571.0;
    markers = koki_find_markers(koki, bw, 0.11, &params);

    assert(markers != NULL);
    cvRelease ((void**)&bw);
    cvRelease ((void**)&color);
    cvRelease ((void**)&m);

    return markers;
}

static Handle<Object> markerToV8( const koki_marker_t* marker ){
    Local<Object> info = Object::New();
    info->Set(String::NewSymbol("code"), Integer::NewFromUnsigned(marker->code)->ToUint32());

    Local<Object> centre = Object::New();
    Local<Object> image = Object::New();
    image->Set(String::NewSymbol("x"), Number::New(marker->centre.image.x));
    image->Set(String::NewSymbol("y"), Number::New(marker->centre.image.y));
    centre->Set(String::NewSymbol("image"), image);
    Local<Object> world = Object::New();
    world->Set(String::NewSymbol("x"), Number::New(marker->centre.world.x));
    world->Set(String::NewSymbol("y"), Number::New(marker->centre.world.y));
    world->Set(String::NewSymbol("y"), Number::New(marker->centre.world.z));
    centre->Set(String::NewSymbol("world"), world);
    info->Set(String::NewSymbol("centre"), centre);

    Local<Object> rotation = Object::New();
    rotation->Set(String::NewSymbol("x"), Number::New(marker->rotation.x));
    rotation->Set(String::NewSymbol("y"), Number::New(marker->rotation.y));
    rotation->Set(String::NewSymbol("y"), Number::New(marker->rotation.z));
    info->Set(String::NewSymbol("rotation"), rotation);

    Local<Object> bearing = Object::New();
    bearing->Set(String::NewSymbol("x"), Number::New(marker->bearing.x));
    bearing->Set(String::NewSymbol("y"), Number::New(marker->bearing.y));
    bearing->Set(String::NewSymbol("y"), Number::New(marker->bearing.z));
    info->Set(String::NewSymbol("bearing"), bearing);

    Local<Object> vertices = Array::New(4);
    for(unsigned int j=0;j<4;j++) {
        Local<Object> vertex = Object::New();
        vertex->Set(String::NewSymbol("x"),
                Number::New(marker->vertices[j].world.x));
        vertex->Set(String::NewSymbol("y"),
                Number::New(marker->vertices[j].world.y));
        vertex->Set(String::NewSymbol("y"),
                Number::New(marker->vertices[j].world.z));
        vertices->Set(j, vertex);
    }
    info->Set(String::NewSymbol("vertices"), vertices);

    return info;
}

struct Baton {
    uv_work_t request;
    Persistent<Function> callback;
    int error_code;
    std::string error_message;

    Persistent<Object> buffer;
    unsigned char* bytes;
    int length;
    GPtrArray* markers;
};

void AsyncWork(uv_work_t* req) {
    Baton* baton = static_cast<Baton*>(req->data);
    baton->markers = process_jpeg( baton->bytes, baton->length );
}

void AsyncAfter(uv_work_t* req) {
    HandleScope scope;
    Baton* baton = static_cast<Baton*>(req->data);

    if (baton->error_code) {
    } else {
        Local<Array> results = Array::New(baton->markers->len);
        for (unsigned int i=0; i<baton->markers->len; i++){
            koki_marker_t *marker = reinterpret_cast<koki_marker_t*>(g_ptr_array_index(baton->markers, i));
            results->Set(i, markerToV8( marker ));
        }
        koki_markers_free(baton->markers);

        Local<Value> argv[] = {
            Local<Value>::New(Null()),
            results
        };
        baton->callback->Call(Context::GetCurrent()->Global(), 2, argv);
    }

    baton->callback.Dispose();
    baton->buffer.Dispose();
    delete baton;
}

static Handle<Value> findMarkers(const Arguments& args)
{
    HandleScope scope;
    if (args.Length() != 2) {
        return ThrowException(Exception::Error(String::New("Wrong number of arguments")));
    }
    if (!Buffer::HasInstance(args[0])) {
        return ThrowException(Exception::TypeError(String::New("Wrong argument type (expect buffer)")));
    }
    if (!args[1]->IsFunction()) {
        return ThrowException(Exception::TypeError(String::New("Wrong argument type (expect function)")));
    }
    Local<Object> jpeg = args[0]->ToObject();
    Local<Function> callback = Local<Function>::Cast(args[1]);

    Baton* baton = new Baton();
    baton->request.data = baton;
    baton->callback = Persistent<Function>::New(callback);

    baton->buffer = Persistent<Object>::New(jpeg);
    baton->bytes = reinterpret_cast<unsigned char*>(Buffer::Data(jpeg));
    baton->length = Buffer::Length(jpeg);

    uv_queue_work(uv_default_loop(),
            &baton->request,
            AsyncWork, AsyncAfter);
    return Undefined();
}

extern "C" void init(Handle<Object> target)
{
    koki = koki_new();

    target->Set(String::NewSymbol("findMarkers"),
            FunctionTemplate::New(findMarkers)->GetFunction());
}

NODE_MODULE(koki, init)
