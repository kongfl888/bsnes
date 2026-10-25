auto DriverSettings::create() -> void {
  setCollapsible();
  setVisible(false);

  videoLabel.setText("视频").setFont(Font().setBold());
  videoDriverLabel.setText("驱动：");
  videoDriverOption.onChange([&] {
    videoDriverUpdate.setText(videoDriverOption.selected().text() != video.driver() ? "更改" : "重载");
  });
  videoDriverUpdate.setText("更改").onActivate([&] { videoDriverChange(); });
  videoMonitorLabel.setText("全屏显示器：").setToolTip(
    "设置全屏时输出到哪块显示器。"
  );
  videoMonitorOption.onChange([&] { videoMonitorChange(); });
  videoFormatLabel.setText("格式：");
  videoFormatOption.onChange([&] { videoFormatChange(); });
  videoExclusiveToggle.setText("独占模式").setToolTip(
    "全屏模式独占显示器。\n"
    "这使得自适应同步效果更佳，并减少了输入延迟。\n"
    "然而，使用多台显示器的用户应关闭此选项。\n"
    "注意：Direct3D 独占模式也不支持所请求的显示器。"
  ).onToggle([&] {
    settings.video.exclusive = videoExclusiveToggle.checked();
    program.updateVideoExclusive();
  });
  videoBlockingToggle.setText("同步").setToolTip(
    "渲染帧前等待显卡就绪。\n"
    "消除丢失或重复的帧；但可能会使音频失真。\n\n"
    "使用此选项时，建议禁用音频同步，\n"
    "并实现动态速率控制。或者\n"
    "调整音频偏斜选项以减少缓冲区下溢/上溢。"
  ).onToggle([&] {
    settings.video.blocking = videoBlockingToggle.checked();
    program.updateVideoBlocking();
    presentation.speedMenu.setEnabled(!videoBlockingToggle.checked() && audioBlockingToggle.checked());
  });
  videoFlushToggle.setText("GPU同步").setToolTip({
    "（仅OpenGL驱动）\n\n"
    "使GPU等待，直到帧完全渲染完毕。\n"
    "在最好的情况下，这可以消除最多一帧的输入延迟。\n"
    "然而，这会带来大约20%的性能损失。\n\n"
    "除非有必要，否则你应该禁用此选项。"
  }).onToggle([&] {
    settings.video.flush = videoFlushToggle.checked();
    program.updateVideoFlush();
  });
  videoSpacer.setColor({192, 192, 192});

  audioLabel.setText("音频").setFont(Font().setBold());
  audioDriverLabel.setText("驱动：");
  audioDriverOption.onChange([&] {
    audioDriverUpdate.setText(audioDriverOption.selected().text() != audio.driver() ? "更改" : "重载");
  });
  audioDriverUpdate.setText("更改").onActivate([&] { audioDriverChange(); });
  audioDeviceLabel.setText("输出设备：");
  audioDeviceOption.onChange([&] { audioDeviceChange(); });
  audioFrequencyLabel.setText("采样率：");
  audioFrequencyOption.onChange([&] { audioFrequencyChange(); });
  audioLatencyLabel.setText("延迟：");
  audioLatencyOption.onChange([&] { audioLatencyChange(); });
  audioExclusiveToggle.setText("独占模式").setToolTip(
    "（仅WASAPI驱动）\n\n"
    "获得声卡设备的独占控制权。\n"
    "这可以显著降低音频延迟。\n"
    "但是，它会屏蔽所有其他应用程序的声音。"
  ).onToggle([&] {
    settings.audio.exclusive = audioExclusiveToggle.checked();
    program.updateAudioExclusive();
  });
  audioBlockingToggle.setText("同步").setToolTip(
    "输出音频采样前等待声卡就绪。\n"
    "Eliminates audio distortio; but can distort video.\n\n"
    "With this option, it's recommended to disable video sync.\n"
    "For best results, use this with an adaptive sync monitor."
  ).onToggle([&] {
    settings.audio.blocking = audioBlockingToggle.checked();
    program.updateAudioBlocking();
    presentation.speedMenu.setEnabled(!videoBlockingToggle.checked() && audioBlockingToggle.checked());
  });
  audioDynamicToggle.setText("动态速率").setToolTip(
    "（仅OSS、XAudio2、waveOut驱动）\n\n"
    "以微小的幅度动态调整音频频率。\n"
    "在启用视频同步且禁用音频同步的情况下使用此功能。\n\n"
    "这可以制作出画面极其流畅、音质清晰的视频，\n"
    "但前提是你的显示器刷新率设置正确：\n"
    "NTSC制式的游戏为60赫兹，PAL制式的游戏为50赫兹。"
  ).onToggle([&] {
    settings.audio.dynamic = audioDynamicToggle.checked();
    program.updateAudioDynamic();
  });
  audioSpacer.setColor({192, 192, 192});

  inputLabel.setText("输入").setFont(Font().setBold());
  inputDriverLabel.setText("驱动：");
  inputDriverOption.onChange([&] {
    inputDriverUpdate.setText(inputDriverOption.selected().text() != input.driver() ? "更改" : "重载");
  });
  inputDriverUpdate.setText("更改").setToolTip(
    "重新加载驱动可识别热插拔设备。\n"
    "这对于缺乏自动热插拔支持的API非常有用，\n"
    "例如DirectInput和SDL。"
  ).onActivate([&] { inputDriverChange(); });
  inputSpacer.setColor({192, 192, 192});

  syncModeLabel.setText("同步模式预设：").setFont(Font().setBold());
  syncModeRequirements.setText(
    "自适应同步：需要G-Sync或FreeSync显示器。\n"
    "动态速率控制：要求显示器和SNES（任天堂超级任天堂娱乐系统）的刷新率相匹配。"
  );
  adaptiveSyncMode.setText("自适应同步").onActivate([&] {
    if(!audioBlockingToggle.enabled()) {
      return (void)MessageDialog().setAlignment(settingsWindow).setTitle("失败").setText({
        "当前驱动配置不支持自适应同步。\n"
        "自适应同步需要音频同步支持。"
      }).error();
    }

    if(videoExclusiveToggle.enabled() && !videoExclusiveToggle.checked()) videoExclusiveToggle.setChecked(true).doToggle();
    if(videoBlockingToggle.enabled() && videoBlockingToggle.checked()) videoBlockingToggle.setChecked(false).doToggle();
    if(audioBlockingToggle.enabled() && !audioBlockingToggle.checked()) audioBlockingToggle.setChecked(true).doToggle();
    if(audioDynamicToggle.enabled() && audioDynamicToggle.checked()) audioDynamicToggle.setChecked(false).doToggle();

    MessageDialog().setAlignment(settingsWindow).setTitle("成功").setText({
      "自适应同步在全屏独占模式下效果最佳。\n"
      "使用系统能够管理的最低音频延迟设置。\n"
      "需要G-sync或FreeSync显示器。\n"
      "您必须在驱动程序设置面板中启用自适应同步。"
    }).information();
  });
  dynamicRateControlMode.setText("动态速率控制").onActivate([&] {
    if(!videoBlockingToggle.enabled() || !audioDynamicToggle.enabled()) {
      return (void)MessageDialog().setAlignment(settingsWindow).setTitle("失败").setText({
        "当前驱动配置不支持动态速率控制。\n"
        "动态码率控制需要视频同步和音频动态码率支持。"
      }).error();
    }

    if(videoBlockingToggle.enabled() && !videoBlockingToggle.checked()) videoBlockingToggle.setChecked(true).doToggle();
    if(audioExclusiveToggle.enabled() && !audioExclusiveToggle.checked()) audioExclusiveToggle.setChecked(true).doToggle();
    if(audioBlockingToggle.enabled() && audioBlockingToggle.checked()) audioBlockingToggle.setChecked(false).doToggle();
    if(audioDynamicToggle.enabled() && !audioDynamicToggle.checked()) audioDynamicToggle.setChecked(true).doToggle();

    MessageDialog().setAlignment(settingsWindow).setTitle("成功").setText({
      "动态速率控制要求显示器运行在：\n"
      "NTSC制式游戏采用60Hz刷新率，PAL制式游戏采用50Hz刷新率。\n"
      "使用系统能够管理的最低音频延迟设置。"
    }).information();
  });
}

//

auto DriverSettings::videoDriverChanged() -> void {
  videoDriverOption.reset();
  for(auto& driver : video.hasDrivers()) {
    ComboButtonItem item{&videoDriverOption};
    item.setText(driver);
    if(driver == video.driver()) item.setSelected();
  }
  videoDriverActive.setText({"当前驱动： ", video.driver()});
  videoDriverOption.doChange();
  videoMonitorChanged();
  videoFormatChanged();
  videoExclusiveToggle.setChecked(video.exclusive()).setEnabled(video.hasExclusive());
  videoBlockingToggle.setChecked(video.blocking()).setEnabled(video.hasBlocking());
  videoFlushToggle.setChecked(video.flush()).setEnabled(video.hasFlush());
  setGeometry(geometry());
}

auto DriverSettings::videoDriverChange() -> void {
  auto item = videoDriverOption.selected();
  settings.video.driver = item.text();
  if(!emulator->loaded() || item.text() == "None" || MessageDialog(
    "警告：不兼容的驱动程序可能会导致 bsnes 崩溃。\n"
    "为了安全起见，强烈建议您先关闭游戏。\n"
    "您现在是否仍希望继续进行视频驱动程序的更改？"
  ).setAlignment(*settingsWindow).question() == "Yes") {
    program.save();
    program.saveUndoState();
    settings.general.crashed = true;
    settings.save();
    program.updateVideoDriver(settingsWindow);
    settings.general.crashed = false;
    settings.save();
    videoDriverChanged();
    if(settings.emulator.hack.ppu.hd
    && settings.emulator.hack.ppu.hdMode7.gpuSupersample
    && !program.videoSupportsHdGpu()) {
      program.selectFastPpuDueToDriver(settingsWindow);
    }
  }
}

auto DriverSettings::videoMonitorChanged() -> void {
  videoMonitorOption.reset();
  for(auto& monitor : Video::hasMonitors()) {
    ComboButtonItem item{&videoMonitorOption};
    item.setText(monitor.name);
    if(monitor.name == video.monitor()) item.setSelected();
  }
  videoMonitorOption.setEnabled(videoMonitorOption.itemCount() > 1);
  setGeometry(geometry());
  videoMonitorChange();
}

auto DriverSettings::videoMonitorChange() -> void {
  auto item = videoMonitorOption.selected();
  settings.video.monitor = item.text();
  program.updateVideoMonitor();
}

auto DriverSettings::videoFormatChanged() -> void {
  videoFormatOption.reset();
  for(auto& format : video.hasFormats()) {
    ComboButtonItem item{&videoFormatOption};
    item.setText(format);
    if(format == video.format()) item.setSelected();
  }
  videoFormatOption.setEnabled(videoFormatOption.itemCount() > 1);
  setGeometry(geometry());
  videoFormatChange();
}

auto DriverSettings::videoFormatChange() -> void {
  auto item = videoFormatOption.selected();
  settings.video.format = item.text();
  program.updateVideoFormat();
}

//

auto DriverSettings::audioDriverChanged() -> void {
  audioDriverOption.reset();
  for(auto& driver : audio.hasDrivers()) {
    ComboButtonItem item{&audioDriverOption};
    item.setText(driver);
    if(driver == audio.driver()) item.setSelected();
  }
  audioDriverActive.setText({"当前驱动： ", audio.driver()});
  audioDriverOption.doChange();
  audioDeviceChanged();
  audioFrequencyChanged();
  audioLatencyChanged();
  audioExclusiveToggle.setChecked(audio.exclusive()).setEnabled(audio.hasExclusive());
  audioBlockingToggle.setChecked(audio.blocking()).setEnabled(audio.hasBlocking());
  audioDynamicToggle.setChecked(audio.dynamic()).setEnabled(audio.hasDynamic());
  setGeometry(geometry());
}

auto DriverSettings::audioDriverChange() -> void {
  auto item = audioDriverOption.selected();
  settings.audio.driver = item.text();
  if(!emulator->loaded() || item.text() == "None" || MessageDialog(
    "警告：不兼容的驱动程序可能会导致 bsnes 崩溃。\n"
    "为了安全起见，强烈建议您先关闭游戏。\n"
    "您现在是否仍希望继续进行音频驱动程序的更改？"
  ).setAlignment(*settingsWindow).question() == "Yes") {
    program.save();
    program.saveUndoState();
    settings.general.crashed = true;
    settings.save();
    program.updateAudioDriver(settingsWindow);
    settings.general.crashed = false;
    settings.save();
    audioDriverChanged();
  }
}

auto DriverSettings::audioDeviceChanged() -> void {
  audioDeviceOption.reset();
  for(auto& device : audio.hasDevices()) {
    ComboButtonItem item{&audioDeviceOption};
    item.setText(device);
    if(device == audio.device()) item.setSelected();
  }
//audioDeviceOption.setEnabled(audio->hasDevice());
  setGeometry(geometry());
}

auto DriverSettings::audioDeviceChange() -> void {
  auto item = audioDeviceOption.selected();
  settings.audio.device = item.text();
  program.updateAudioDevice();
  audioFrequencyChanged();
  audioLatencyChanged();
}

auto DriverSettings::audioFrequencyChanged() -> void {
  audioFrequencyOption.reset();
  for(auto& frequency : audio.hasFrequencies()) {
    ComboButtonItem item{&audioFrequencyOption};
    item.setText({frequency, " Hz"});
    if(frequency == audio.frequency()) item.setSelected();
  }
//audioFrequencyOption.setEnabled(audio->hasFrequency());
  setGeometry(geometry());
}

auto DriverSettings::audioFrequencyChange() -> void {
  auto item = audioFrequencyOption.selected();
  settings.audio.frequency = item.text().natural();
  program.updateAudioFrequency();
}

auto DriverSettings::audioLatencyChanged() -> void {
  audioLatencyOption.reset();
  for(auto& latency : audio.hasLatencies()) {
    ComboButtonItem item{&audioLatencyOption};
    item.setText(latency);
    if(latency == audio.latency()) item.setSelected();
  }
//audioLatencyOption.setEnabled(audio->hasLatency());
  setGeometry(geometry());
}

auto DriverSettings::audioLatencyChange() -> void {
  auto item = audioLatencyOption.selected();
  settings.audio.latency = item.text().natural();
  program.updateAudioLatency();
}

//

auto DriverSettings::inputDriverChanged() -> void {
  inputDriverOption.reset();
  for(auto& driver : input.hasDrivers()) {
    ComboButtonItem item{&inputDriverOption};
    item.setText(driver);
    if(driver == input.driver()) item.setSelected();
  }
  inputDriverActive.setText({"当前驱动： ", input.driver()});
  inputDriverOption.doChange();
  setGeometry(geometry());
}

auto DriverSettings::inputDriverChange() -> void {
  auto item = inputDriverOption.selected();
  settings.input.driver = item.text();
  if(!emulator->loaded() || item.text() == "None" || MessageDialog(
    "警告：不兼容的驱动程序可能会导致 bsnes 崩溃。\n"
    "为了安全起见，强烈建议您先关闭游戏。\n"
    "您是否仍希望立即继续进行输入驱动程序的更改？"
  ).setAlignment(*settingsWindow).question() == "Yes") {
    program.save();
    program.saveUndoState();
    settings.general.crashed = true;
    settings.save();
    program.updateInputDriver(settingsWindow);
    settings.general.crashed = false;
    settings.save();
    inputDriverChanged();
  }
}
