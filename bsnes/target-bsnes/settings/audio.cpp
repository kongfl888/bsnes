auto AudioSettings::create() -> void {
  setCollapsible();
  setVisible(false);

  effectsLabel.setFont(Font().setBold()).setText("效果");
  effectsLayout.setSize({3, 3});
  effectsLayout.column(0).setAlignment(1.0);
  skewLabel.setText("偏移：").setToolTip(
    "以赫兹为单位调整音频频率偏移量。\n\n"
    "这本质上是静态速率控制：\n"
    "首先，启用视频和音频同步功能。\n"
    "然后，调整这个值的大小，以尝试减少错误。\n"
    "一个方向会促进视频的发展，但会损害音频。\n"
    "另一个方向则相反。\n"
    "我们的想法是找到最佳的折中方案。\n\n"
    "使用动态速率控制时，应将其设置为0。"
  );
  skewValue.setAlignment(0.5).setToolTip(skewLabel.toolTip());
  skewSlider.setLength(10001).setPosition(settings.audio.skew + 5000).onChange([&] {
    string value = {skewSlider.position() > 5000 ? "+" : "", (int)skewSlider.position() - 5000};
    settings.audio.skew = value.integer();
    skewValue.setText(value);
    if(audio.driver() != "None") program.updateAudioFrequency();
  }).doChange();
  volumeLabel.setText("音量：").setToolTip(
    "调整音频输出音量。\n\n"
    "如果可能的话，你不应使用超过100%的值！\n"
    "如果这样做，可能会发生音频削波失真。"
  );
  volumeValue.setAlignment(0.5).setToolTip(volumeLabel.toolTip());
  volumeSlider.setLength(201).setPosition(settings.audio.volume).onChange([&] {
    string value = {volumeSlider.position(), "%"};
    settings.audio.volume = value.natural();
    volumeValue.setText(value);
    program.updateAudioEffects();
  }).doChange();
  balanceLabel.setText("平衡：").setToolTip(
    "音频声像，数值越小偏左，越大偏右。\n\n"
    "建议设置为50%（居中）。"
  );
  balanceValue.setAlignment(0.5).setToolTip(balanceLabel.toolTip());
  balanceSlider.setLength(101).setPosition(settings.audio.balance).onChange([&] {
    string value = {balanceSlider.position(), "%"};
    settings.audio.balance = value.natural();
    balanceValue.setText(value);
    program.updateAudioEffects();
  }).doChange();

  muteUnfocused.setText("失去焦点时静音").setChecked(settings.audio.muteUnfocused).onToggle([&] {
    settings.audio.muteUnfocused = muteUnfocused.checked();
  });
}
