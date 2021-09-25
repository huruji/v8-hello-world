#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "deps/v8/include/libplatform/libplatform.h"
#include "deps/v8/include/v8.h"
using namespace v8;


class App {
public:
    std::unique_ptr<v8::Platform> platform;
    Isolate::CreateParams create_params;
    Isolate* isolate;
    v8::Local<v8::Context> context;
public:
    void createPlatform(char* argv[]) {
        // v8 初始化
        V8::InitializeICUDefaultLocation(argv[0]);
        V8::InitializeExternalStartupData(argv[0]);
        this->platform = v8::platform::NewDefaultPlatform();
        V8::InitializePlatform(this->platform.get());
        V8::Initialize();
    }

    void createVM() {
        // 创建虚拟机
        this->create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();;
        this->isolate = v8::Isolate::New(this->create_params);
    }

    void ShutdownVM() {
        // 关闭虚拟机
        this->isolate->Dispose();
        V8::Dispose();
        V8::ShutdownPlatform();
        delete this->create_params.array_buffer_allocator;
    }

    Isolate* getIsolate() {
        return this->isolate;
    }

    void enterScope() {
        v8::Isolate::Scope isolate_scope(this->isolate);
        v8::HandleScope handle_scope(this->isolate);
        this->context = v8::Context::New(this->isolate);
        v8::Context::Scope context_scope(this->context);
    }


    void runScriptString(const char* scriptString) {
        // 进入 scope
        v8::Isolate::Scope isolate_scope(this->isolate);
        v8::HandleScope handle_scope(this->isolate);
        this->context = v8::Context::New(this->isolate);
        v8::Context::Scope context_scope(this->context);
        v8::Local<v8::String> source =
                v8::String::NewFromUtf8(this->isolate, scriptString,
                                        v8::NewStringType::kNormal)
                        .ToLocalChecked();
        // 编译运行
        v8::Local<v8::Script> script =
                v8::Script::Compile(context, source).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
        // 将结果转化为字符串
        v8::String::Utf8Value utf8(this->isolate, result);
        printf("%s\n", *utf8);
    }

};


int main(int argc, char* argv[]) {
    App app;
    app.createPlatform(argv);
    app.createVM();
    const char *js = "100 + 100 + ' string' + true";
    app.runScriptString(js);

    app.ShutdownVM();
    return 0;
}