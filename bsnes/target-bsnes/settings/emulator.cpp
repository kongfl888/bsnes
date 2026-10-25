auto EmulatorSettings::create() -> void {
  setCollapsible();
  setVisible(false);

  optionsLabel.setText("选项").setFont(Font().setBold());
  warnOnUnverifiedGames.setText("加载未验证游戏时提示警告").setChecked(settings.emulator.warnOnUnverifiedGames).onToggle([&] {
    settings.emulator.warnOnUnverifiedGames = warnOnUnverifiedGames.checked();
  });
  autoSaveMemory.setText("定期自动保存内存").setChecked(settings.emulator.autoSaveMemory.enable).onToggle([&] {
    settings.emulator.autoSaveMemory.enable = autoSaveMemory.checked();
  });
  autoSaveStateOnUnload.setText("关闭游戏时自动保存 undo 快照").setChecked(settings.emulator.autoSaveStateOnUnload).onToggle([&] {
    settings.emulator.autoSaveStateOnUnload = autoSaveStateOnUnload.checked();
    if(!autoSaveStateOnUnload.checked()) {
      autoLoadStateOnLoad.setEnabled(false).setChecked(false).doToggle();
    } else {
      autoLoadStateOnLoad.setEnabled(true);
    }
  }).doToggle();
  autoLoadStateOnLoad.setText("加载后自动继续运行").setChecked(settings.emulator.autoLoadStateOnLoad).onToggle([&] {
    settings.emulator.autoLoadStateOnLoad = autoLoadStateOnLoad.checked();
  });
  nativeFileDialogs.setText("使用系统原生文件对话框").setChecked(settings.general.nativeFileDialogs).onToggle([&] {
    settings.general.nativeFileDialogs = nativeFileDialogs.checked();
  });
  screenSaver.setText("模拟器运行时允许屏幕保护").setChecked(settings.general.screenSaver).onToggle([&] {
    settings.general.screenSaver = screenSaver.checked();
    // setting can be toggled while emulation is active
    if(!program.inactive()) Application::setScreenSaver(settings.general.screenSaver);
  });
  optionsSpacer.setColor({192, 192, 192});

  fastForwardLabel.setText("快进").setFont(Font().setBold());

  frameSkipLabel.setText("跳帧：").setToolTip({
    "设置快进时跳过的帧数。\n"
    "Frame skipping allows a higher maximum fast forwarding frame rate."
  });

  frameSkipAmount.append(ComboButtonItem().setText("无"));
  frameSkipAmount.append(ComboButtonItem().setText("1帧"));
  frameSkipAmount.append(ComboButtonItem().setText("2帧"));
  frameSkipAmount.append(ComboButtonItem().setText("3帧"));
  frameSkipAmount.append(ComboButtonItem().setText("4帧"));
  frameSkipAmount.append(ComboButtonItem().setText("5帧"));
  frameSkipAmount.append(ComboButtonItem().setText("6帧"));
  frameSkipAmount.append(ComboButtonItem().setText("7帧"));
  frameSkipAmount.append(ComboButtonItem().setText("8帧"));
  frameSkipAmount.append(ComboButtonItem().setText("9帧"));
  frameSkipAmount.item(settings.fastForward.frameSkip).setSelected();
  frameSkipAmount.onChange([&] {
    settings.fastForward.frameSkip = frameSkipAmount.selected().offset();
  });

  limiterLabel.setText("控速：").setToolTip({
    "设置快进的最高速度上限。"
  });

  limiterAmount.append(ComboButtonItem().setText("无"));
  limiterAmount.append(ComboButtonItem().setText("200%"));
  limiterAmount.append(ComboButtonItem().setText("300%"));
  limiterAmount.append(ComboButtonItem().setText("400%"));
  limiterAmount.append(ComboButtonItem().setText("500%"));
  limiterAmount.append(ComboButtonItem().setText("600%"));
  limiterAmount.append(ComboButtonItem().setText("700%"));
  limiterAmount.append(ComboButtonItem().setText("800%"));
  if(settings.fastForward.limiter == 0) limiterAmount.item(0).setSelected();
  if(settings.fastForward.limiter == 2) limiterAmount.item(1).setSelected();
  if(settings.fastForward.limiter == 3) limiterAmount.item(2).setSelected();
  if(settings.fastForward.limiter == 4) limiterAmount.item(3).setSelected();
  if(settings.fastForward.limiter == 5) limiterAmount.item(4).setSelected();
  if(settings.fastForward.limiter == 6) limiterAmount.item(5).setSelected();
  if(settings.fastForward.limiter == 7) limiterAmount.item(6).setSelected();
  if(settings.fastForward.limiter == 8) limiterAmount.item(7).setSelected();
  limiterAmount.onChange([&] {
    auto index = limiterAmount.selected().offset();
    if(index == 0) settings.fastForward.limiter = 0;
    if(index == 1) settings.fastForward.limiter = 2;
    if(index == 2) settings.fastForward.limiter = 3;
    if(index == 3) settings.fastForward.limiter = 4;
    if(index == 4) settings.fastForward.limiter = 5;
    if(index == 5) settings.fastForward.limiter = 6;
    if(index == 6) settings.fastForward.limiter = 7;
    if(index == 7) settings.fastForward.limiter = 8;
  });

  fastForwardMute.setText("快进时静音").setChecked(settings.fastForward.mute).onToggle([&] {
    settings.fastForward.mute = fastForwardMute.checked();
  });

  fastForwardSpacer.setColor({192, 192, 192});

  rewindLabel.setText("倒带").setFont(Font().setBold());

  rewindFrequencyLabel.setText("频率：");
  rewindFrequencyOption.append(ComboButtonItem().setText("关闭"));
  rewindFrequencyOption.append(ComboButtonItem().setText("每10帧"));
  rewindFrequencyOption.append(ComboButtonItem().setText("每20帧"));
  rewindFrequencyOption.append(ComboButtonItem().setText("每30帧"));
  rewindFrequencyOption.append(ComboButtonItem().setText("每40帧"));
  rewindFrequencyOption.append(ComboButtonItem().setText("每50帧"));
  rewindFrequencyOption.append(ComboButtonItem().setText("每60帧"));
  if(settings.rewind.frequency ==  0) rewindFrequencyOption.item(0).setSelected();
  if(settings.rewind.frequency == 10) rewindFrequencyOption.item(1).setSelected();
  if(settings.rewind.frequency == 20) rewindFrequencyOption.item(2).setSelected();
  if(settings.rewind.frequency == 30) rewindFrequencyOption.item(3).setSelected();
  if(settings.rewind.frequency == 40) rewindFrequencyOption.item(4).setSelected();
  if(settings.rewind.frequency == 50) rewindFrequencyOption.item(5).setSelected();
  if(settings.rewind.frequency == 60) rewindFrequencyOption.item(6).setSelected();
  rewindFrequencyOption.onChange([&] {
    settings.rewind.frequency = rewindFrequencyOption.selected().offset() * 10;
    program.rewindReset();
  });

  rewindLengthLabel.setText("时长：");
  rewindLengthOption.append(ComboButtonItem().setText( "10 states"));
  rewindLengthOption.append(ComboButtonItem().setText( "20 states"));
  rewindLengthOption.append(ComboButtonItem().setText( "40 states"));
  rewindLengthOption.append(ComboButtonItem().setText( "80 states"));
  rewindLengthOption.append(ComboButtonItem().setText("160 states"));
  rewindLengthOption.append(ComboButtonItem().setText("320 states"));
  if(settings.rewind.length ==  10) rewindLengthOption.item(0).setSelected();
  if(settings.rewind.length ==  20) rewindLengthOption.item(1).setSelected();
  if(settings.rewind.length ==  40) rewindLengthOption.item(2).setSelected();
  if(settings.rewind.length ==  80) rewindLengthOption.item(3).setSelected();
  if(settings.rewind.length == 160) rewindLengthOption.item(4).setSelected();
  if(settings.rewind.length == 320) rewindLengthOption.item(5).setSelected();
  rewindLengthOption.onChange([&] {
    settings.rewind.length = 10 << rewindLengthOption.selected().offset();
    program.rewindReset();
  });

  rewindMute.setText("倒带时静音").setChecked(settings.rewind.mute).onToggle([&] {
    settings.rewind.mute = rewindMute.checked();
  });
}
