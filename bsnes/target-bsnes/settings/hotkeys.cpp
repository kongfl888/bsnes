auto HotkeySettings::create() -> void {
  setCollapsible();
  setVisible(false);

  mappingList.setBatchable();
  mappingList.setHeadered();
  mappingList.onActivate([&](auto cell) { assignMapping(cell); });
  mappingList.onChange([&] {
    auto batched = mappingList.batched();
    assignButton.setEnabled(batched.size() == 1);
    clearButton.setEnabled(batched.size() >= 1);
  });
  mappingList.onSize([&] {
    mappingList.resizeColumns();
  });
  logicLabel.setText("组合逻辑：").setToolTip(
    "设置热键触发条件：全部按键按下 或 任意按键按下。\n"
    "Use 'and' logic if you want keyboard combinations such as Ctrl+F to trigger a hotkey.\n"
    "Use 'or' logic if you want both a keyboard and joypad to trigger the same hotkey."
  );
  logicAND.setText("and").setToolTip("需要按下所有绑定按键才能触发热键。").onActivate([&] {
    settings.input.hotkey.logic = "and";
    inputManager.hotkeyLogic = InputMapping::Logic::AND;
  });
  logicOR.setText("or").setToolTip("按下任意一个绑定按键即可触发热键。").onActivate([&] {
    settings.input.hotkey.logic = "or";
    inputManager.hotkeyLogic = InputMapping::Logic::OR;
  });
  if(settings.input.hotkey.logic == "and") logicAND.setChecked().doActivate();
  if(settings.input.hotkey.logic == "or") logicOR.setChecked().doActivate();
  inputSink.setFocusable();
  assignButton.setText("绑定").onActivate([&] {
    clearButton.doActivate();
    assignMapping(mappingList.selected().cell(0));
  });
  clearButton.setText("清空").onActivate([&] {
    cancelMapping();
    for(auto mapping : mappingList.batched()) {
      auto& hotkey = inputManager.hotkeys[mapping.offset()];
      for(uint index : range(BindingLimit)) {
        hotkey.unbind(index);
      }
    }
    refreshMappings();
  });
}

auto HotkeySettings::reloadMappings() -> void {
  mappingList.reset();
  mappingList.append(TableViewColumn().setText("名称"));
  for(uint index : range(BindingLimit)) {
    mappingList.append(TableViewColumn().setText({"绑定项 #", 1 + index}).setExpandable());
  }
  for(auto& hotkey : inputManager.hotkeys) {
    TableViewItem item{&mappingList};
    item.append(TableViewCell().setText(hotkey.name).setFont(Font().setBold()));
    for(uint index : range(BindingLimit)) item.append(TableViewCell());
  }
  refreshMappings();
  mappingList.doChange();
}

auto HotkeySettings::refreshMappings() -> void {
  uint item = 0;
  for(auto& hotkey : inputManager.hotkeys) {
    for(uint index : range(BindingLimit)) {
      auto cell = mappingList.item(item).cell(1 + index);
      auto& binding = hotkey.bindings[index];
      cell.setIcon(binding.icon());
      cell.setText(binding.name());
    }
    item++;
  }
  Application::processEvents();
  mappingList.resizeColumns();
}

auto HotkeySettings::assignMapping(TableViewCell cell) -> void {
  inputManager.poll();  //clear any pending events first

  for(auto mapping : mappingList.batched()) {
    activeMapping = inputManager.hotkeys[mapping.offset()];
    activeBinding = max(0, (int)cell.offset() - 1);
    mappingList.item(mapping.offset()).cell(1 + activeBinding).setIcon(Icon::Go::Right).setText("（绑定...）");
    settingsWindow.statusBar.setText({"请按下分配给绑定项的按键：", 1 + activeBinding, " [", activeMapping->name, "] ..."});
    settingsWindow.setDismissable(false);
    inputSink.setFocused();
    return;  //map only one input at a time
  }
}

auto HotkeySettings::cancelMapping() -> void {
  activeMapping.reset();
  settingsWindow.statusBar.setText();
  settingsWindow.doSize();
  settingsWindow.setDismissable(true);
  mappingList.setFocused();
}

auto HotkeySettings::inputEvent(shared_pointer<HID::Device> device, uint group, uint input, int16 oldValue, int16 newValue) -> void {
  if(!activeMapping) return;
  if(device->isMouse()) return;

  if(activeMapping->bind(device, group, input, oldValue, newValue, activeBinding)) {
    activeMapping.reset();
    settingsWindow.statusBar.setText("绑定成功。");
    refreshMappings();
    timer.onActivate([&] {
      timer.setEnabled(false);
      cancelMapping();
    }).setInterval(200).setEnabled();
  }
}
