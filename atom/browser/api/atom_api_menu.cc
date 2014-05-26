// Copyright (c) 2013 GitHub, Inc. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/api/atom_api_menu.h"

#include "atom/browser/api/atom_api_window.h"
#include "atom/browser/ui/accelerator_util.h"
#include "atom/common/native_mate_converters/string16_converter.h"
#include "native_mate/constructor.h"
#include "native_mate/dictionary.h"
#include "native_mate/object_template_builder.h"

#include "atom/common/node_includes.h"

namespace atom {

namespace api {

namespace {

// Call method of delegate object.
v8::Handle<v8::Value> CallDelegate(v8::Handle<v8::Value> default_value,
                                   v8::Handle<v8::Object> menu,
                                   const char* method,
                                   int command_id) {
  v8::Locker locker(node_isolate);
  v8::HandleScope handle_scope(node_isolate);

  v8::Handle<v8::Value> delegate = menu->Get(v8::String::New("delegate"));
  if (!delegate->IsObject())
    return default_value;

  v8::Handle<v8::Function> function = v8::Handle<v8::Function>::Cast(
      delegate->ToObject()->Get(v8::String::New(method)));
  if (!function->IsFunction())
    return default_value;

  v8::Handle<v8::Value> argv = v8::Integer::New(command_id);

  return handle_scope.Close(
      function->Call(v8::Context::GetCurrent()->Global(), 1, &argv));
}

}  // namespace

Menu::Menu()
    : model_(new ui::SimpleMenuModel(this)),
      parent_(NULL) {
}

Menu::~Menu() {
}

bool Menu::IsCommandIdChecked(int command_id) const {
  v8::Locker locker(node_isolate);
  v8::HandleScope handle_scope(node_isolate);
  return CallDelegate(v8::False(),
                      const_cast<Menu*>(this)->GetWrapper(node_isolate),
                      "isCommandIdChecked",
                      command_id)->BooleanValue();
}

bool Menu::IsCommandIdEnabled(int command_id) const {
  v8::Locker locker(node_isolate);
  v8::HandleScope handle_scope(node_isolate);
  return CallDelegate(v8::True(),
                      const_cast<Menu*>(this)->GetWrapper(node_isolate),
                      "isCommandIdEnabled",
                      command_id)->BooleanValue();
}

bool Menu::IsCommandIdVisible(int command_id) const {
  v8::Locker locker(node_isolate);
  v8::HandleScope handle_scope(node_isolate);
  return CallDelegate(v8::True(),
                      const_cast<Menu*>(this)->GetWrapper(node_isolate),
                      "isCommandIdVisible",
                      command_id)->BooleanValue();
}

bool Menu::GetAcceleratorForCommandId(int command_id,
                                      ui::Accelerator* accelerator) {
  v8::Locker locker(node_isolate);
  v8::HandleScope handle_scope(node_isolate);
  v8::Handle<v8::Value> shortcut = CallDelegate(v8::Undefined(),
                                                GetWrapper(node_isolate),
                                                "getAcceleratorForCommandId",
                                                command_id);
  if (shortcut->IsString()) {
    std::string shortcut_str = mate::V8ToString(shortcut);
    return accelerator_util::StringToAccelerator(shortcut_str, accelerator);
  }

  return false;
}

bool Menu::IsItemForCommandIdDynamic(int command_id) const {
  v8::Locker locker(node_isolate);
  v8::HandleScope handle_scope(node_isolate);
  return CallDelegate(v8::False(),
                      const_cast<Menu*>(this)->GetWrapper(node_isolate),
                      "isItemForCommandIdDynamic",
                      command_id)->BooleanValue();
}

string16 Menu::GetLabelForCommandId(int command_id) const {
  v8::Locker locker(node_isolate);
  v8::HandleScope handle_scope(node_isolate);
  v8::Handle<v8::Value> result = CallDelegate(
      v8::False(),
      const_cast<Menu*>(this)->GetWrapper(node_isolate),
      "getLabelForCommandId",
      command_id);
  string16 label;
  mate::ConvertFromV8(node_isolate, result, &label);
  return label;
}

string16 Menu::GetSublabelForCommandId(int command_id) const {
  v8::Locker locker(node_isolate);
  v8::HandleScope handle_scope(node_isolate);
  v8::Handle<v8::Value> result = CallDelegate(
      v8::False(),
      const_cast<Menu*>(this)->GetWrapper(node_isolate),
      "getSubLabelForCommandId",
      command_id);
  string16 label;
  mate::ConvertFromV8(node_isolate, result, &label);
  return label;
}

void Menu::ExecuteCommand(int command_id, int event_flags) {
  v8::Locker locker(node_isolate);
  v8::HandleScope handle_scope(node_isolate);
  CallDelegate(v8::False(), GetWrapper(node_isolate), "executeCommand",
               command_id);
}

void Menu::MenuWillShow(ui::SimpleMenuModel* source) {
  v8::Locker locker(node_isolate);
  v8::HandleScope handle_scope(node_isolate);
  CallDelegate(v8::False(), GetWrapper(node_isolate), "menuWillShow", -1);
}

void Menu::InsertItemAt(
    int index, int command_id, const base::string16& label) {
  model_->InsertItemAt(index, command_id, label);
}

void Menu::InsertSeparatorAt(int index) {
  model_->InsertSeparatorAt(index, ui::NORMAL_SEPARATOR);
}

void Menu::InsertCheckItemAt(int index,
                             int command_id,
                             const base::string16& label) {
  model_->InsertCheckItemAt(index, command_id, label);
}

void Menu::InsertRadioItemAt(int index,
                             int command_id,
                             const base::string16& label,
                             int group_id) {
  model_->InsertRadioItemAt(index, command_id, label, group_id);
}

void Menu::InsertSubMenuAt(int index,
                           int command_id,
                           const base::string16& label,
                           Menu* menu) {
  menu->parent_ = this;
  model_->InsertSubMenuAt(index, command_id, label, menu->model_.get());
}

void Menu::SetSublabel(int index, const base::string16& sublabel) {
  model_->SetSublabel(index, sublabel);
}

void Menu::Clear() {
  model_->Clear();
}

int Menu::GetIndexOfCommandId(int command_id) {
  return model_->GetIndexOfCommandId(command_id);
}

int Menu::GetItemCount() const {
  return model_->GetItemCount();
}

int Menu::GetCommandIdAt(int index) const {
  return model_->GetCommandIdAt(index);
}

base::string16 Menu::GetLabelAt(int index) const {
  return model_->GetLabelAt(index);
}

base::string16 Menu::GetSublabelAt(int index) const {
  return model_->GetSublabelAt(index);
}

bool Menu::IsItemCheckedAt(int index) const {
  return model_->IsItemCheckedAt(index);
}

bool Menu::IsEnabledAt(int index) const {
  return model_->IsEnabledAt(index);
}

bool Menu::IsVisibleAt(int index) const {
  return model_->IsVisibleAt(index);
}

// static
void Menu::BuildPrototype(v8::Isolate* isolate,
                          v8::Handle<v8::ObjectTemplate> prototype) {
  mate::ObjectTemplateBuilder(isolate, prototype)
      .SetMethod("insertItem", &Menu::InsertItemAt)
      .SetMethod("insertCheckItem", &Menu::InsertCheckItemAt)
      .SetMethod("insertRadioItem", &Menu::InsertRadioItemAt)
      .SetMethod("insertSeparator", &Menu::InsertSeparatorAt)
      .SetMethod("insertSubMenu", &Menu::InsertSubMenuAt)
      .SetMethod("setSublabel", &Menu::SetSublabel)
      .SetMethod("clear", &Menu::Clear)
      .SetMethod("getIndexOfCommandId", &Menu::GetIndexOfCommandId)
      .SetMethod("getItemCount", &Menu::GetItemCount)
      .SetMethod("getCommandIdAt", &Menu::GetCommandIdAt)
      .SetMethod("getLabelAt", &Menu::GetLabelAt)
      .SetMethod("getSublabelAt", &Menu::GetSublabelAt)
      .SetMethod("isItemCheckedAt", &Menu::IsItemCheckedAt)
      .SetMethod("isEnabledAt", &Menu::IsEnabledAt)
      .SetMethod("isVisibleAt", &Menu::IsVisibleAt)
#if defined(OS_WIN) || defined(TOOLKIT_GTK)
      .SetMethod("_attachToWindow", &Menu::AttachToWindow)
#endif
#if defined(OS_WIN)
      .SetMethod("_updateStates", &Menu::UpdateStates)
#endif
      .SetMethod("_popup", &Menu::Popup);
}

}  // namespace api

}  // namespace atom


namespace {

void Initialize(v8::Handle<v8::Object> exports) {
  using atom::api::Menu;
  v8::Local<v8::Function> constructor = mate::CreateConstructor<Menu>(
      node_isolate, "Menu", base::Bind(&Menu::Create));
  mate::Dictionary dict(v8::Isolate::GetCurrent(), exports);
  dict.Set("Menu", static_cast<v8::Handle<v8::Value>>(constructor));
#if defined(OS_MACOSX)
  dict.SetMethod("setApplicationMenu", &Menu::SetApplicationMenu);
  dict.SetMethod("sendActionToFirstResponder",
                 &Menu::SendActionToFirstResponder);
#endif
}

}  // namespace

NODE_MODULE(atom_browser_menu, Initialize)
