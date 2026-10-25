auto VideoSettings::create() -> void {
  setCollapsible();
  setVisible(false);

  colorAdjustmentLabel.setFont(Font().setBold()).setText("色彩调节");
  colorLayout.setSize({3, 3});
  colorLayout.column(0).setAlignment(1.0);
  luminanceLabel.setText("亮度：");
  luminanceValue.setAlignment(0.5);
  luminanceSlider.setLength(101).setPosition(settings.video.luminance).onChange([&] {
    string value = {luminanceSlider.position(), "%"};
    settings.video.luminance = value.natural();
    luminanceValue.setText(value);
    program.updateVideoPalette();
  }).doChange();
  saturationLabel.setText("饱和度：");
  saturationValue.setAlignment(0.5);
  saturationSlider.setLength(201).setPosition(settings.video.saturation).onChange([&] {
    string value = {saturationSlider.position(), "%"};
    settings.video.saturation = value.natural();
    saturationValue.setText(value);
    program.updateVideoPalette();
  }).doChange();
  gammaLabel.setText("伽马：");
  gammaValue.setAlignment(0.5);
  gammaSlider.setLength(101).setPosition(settings.video.gamma - 100).onChange([&] {
    string value = {100 + gammaSlider.position(), "%"};
    settings.video.gamma = value.natural();
    gammaValue.setText(value);
    program.updateVideoPalette();
  }).doChange();

  dimmingOption.setText("闲置时画面变暗").setToolTip(
    "模拟暂停时画面变暗以示区分。"
  ).setChecked(settings.video.dimming).onToggle([&] {
    settings.video.dimming = dimmingOption.checked();
  });

  snowOption.setText("闲置时显示雪花噪点").setChecked(settings.video.snow).onToggle([&] {
    settings.video.snow = snowOption.checked();
    presentation.updateProgramIcon();
  });
}
