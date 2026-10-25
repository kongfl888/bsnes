auto CompatibilitySettings::create() -> void {
  setCollapsible();
  setVisible(false);

  entropyLabel.setText("Entropy (随机)").setFont(Font().setBold());
  entropyNone.setText("None").setToolTip(
    "启动时所有内存与寄存器初始化为固定值。\n"
    "使用此选项可兼容非常古老的演示场景自制游戏。"
  ).onActivate([&] {
    settings.emulator.hack.entropy = "None";
  });
  entropyLow.setText("Low").setToolTip(
    "内存使用重复模式随机，寄存器启动时随机。\n"
    "使用这个可以最准确地模拟真实的SNES。"
  ).onActivate([&] {
    settings.emulator.hack.entropy = "Low";
  });
  entropyHigh.setText("High").setToolTip(
    "内存与寄存器尽可能完全随机初始化。\n"
    "在开发新的SNES（任天堂超级任天堂）软件时，请使用此工具以确保与真实硬件的最大兼容性。"
  ).onActivate([&] {
    settings.emulator.hack.entropy = "High";
  });
  if(settings.emulator.hack.entropy == "None") entropyNone.setChecked();
  if(settings.emulator.hack.entropy == "Low" ) entropyLow.setChecked();
  if(settings.emulator.hack.entropy == "High") entropyHigh.setChecked();

  cpuLabel.setFont(Font().setBold()).setText("CPU（处理器）");
  fastMath.setText("快速运算").setToolTip(
    "实机SNES上CPU乘除法需要消耗运算周期。\n"
    "较旧的模拟器无法模拟这些延迟，而是会立即提供结果。\n"
    "一些较旧的ROM破解版不会等待数学运算完成，因此需要使用此破解方法。"
  ).setChecked(settings.emulator.hack.cpu.fastMath).onToggle([&] {
    settings.emulator.hack.cpu.fastMath = fastMath.checked();
    emulator->configure("Hacks/CPU/FastMath", settings.emulator.hack.cpu.fastMath);
  });

  ppuLabel.setFont(Font().setBold()).setText("PPU（图像处理器）");
  noVRAMBlocking.setText("禁用 VRAM 阻塞").setToolTip(
    "该选项模拟旧版ZSNES、Snes9X的BUG：不模拟VRAM阻塞。\n"
    "一些较旧的ROM修改版依赖于这种行为，如果未启用，将无法正确渲染图形。\n"
    "这不仅与真实硬件极度不符，还会影响快速并行处理单元（PPU）的速度。\n"
    "除非你需要玩与bsnes不兼容的游戏，否则请勿启用此选项。"
  ).setChecked(settings.emulator.hack.ppu.noVRAMBlocking).onToggle([&] {
    settings.emulator.hack.ppu.noVRAMBlocking = noVRAMBlocking.checked();
    emulator->configure("Hacks/PPU/NoVRAMBlocking", settings.emulator.hack.ppu.noVRAMBlocking);
  });

  dspLabel.setFont(Font().setBold()).setText("DSP（音频）");
  echoShadow.setText("回声影子RAM").setToolTip(
    "模拟ZSNES的BUG：回声RAM与APU RAM相互独立。\n"
    "许多针对《超级马里奥世界》的较旧ROM修改版都依赖于这一行为，如果不启用这一功能，游戏就会崩溃。\n"
    "然而，它与真实硬件的准确性极差，除非必要，否则不应启用。"
  ).setChecked(settings.emulator.hack.dsp.echoShadow).onToggle([&] {
    settings.emulator.hack.dsp.echoShadow = echoShadow.checked();
    //not a run-time setting: do not call emulator->configure() here.
  });

  note.setText("提示：部分设置需要重载游戏才生效。");
}
