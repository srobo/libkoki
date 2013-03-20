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

static Handle<Value> findMarkers(const Arguments& args)
{
    HandleScope scope;
    if (args.Length() != 1) {
        ThrowException(Exception::Error(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    if (!Buffer::HasInstance(args[0])) {
        ThrowException(Exception::TypeError(String::New("Wrong argument type (expect buffer)")));
        return scope.Close(Undefined());
    }

    Local<Object> jpeg = args[0]->ToObject();
    unsigned char* bytes = reinterpret_cast<unsigned char*>(Buffer::Data(jpeg));
    int length = Buffer::Length(jpeg);
    GPtrArray *markers = process_jpeg( bytes, length );

    Local<Array> results = Array::New(markers->len);
    for (unsigned int i=0; i<markers->len; i++){
        koki_marker_t *marker = reinterpret_cast<koki_marker_t*>(g_ptr_array_index(markers, i));

        Handle<Object> info = Object::New();
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

        results->Set(i, info);
    }
    koki_markers_free(markers);

    return scope.Close(results);
}

extern "C" void init(Handle<Object> target)
{
    koki = koki_new();

    target->Set(String::NewSymbol("findMarkers"),
            FunctionTemplate::New(findMarkers)->GetFunction());
}

NODE_MODULE(koki, init)
