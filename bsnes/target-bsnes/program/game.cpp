auto Program::load() -> void {
  unload();

  emulator->configure("System/Serialization/Method", settings.emulator.serialization.method);
  emulator->configure("Hacks/Hotfixes", settings.emulator.hack.hotfixes);
  emulator->configure("Hacks/Entropy", settings.emulator.hack.entropy);
  emulator->configure("Hacks/CPU/Overclock", settings.emulator.hack.cpu.overclock);
  emulator->configure("Hacks/CPU/FastMath", settings.emulator.hack.cpu.fastMath);
  emulator->configure("Hacks/PPU/Fast", settings.emulator.hack.ppu.fast);
  emulator->configure("Hacks/PPU/HD", settings.emulator.hack.ppu.hd);
  emulator->configure("Hacks/PPU/Deinterlace", settings.emulator.hack.ppu.deinterlace);
  emulator->configure("Hacks/PPU/NoSpriteLimit", settings.emulator.hack.ppu.noSpriteLimit);
  emulator->configure("Hacks/PPU/HD-Deinterlace", settings.emulator.hack.ppu.hdDeinterlace);
  emulator->configure("Hacks/PPU/HD-NoSpriteLimit", settings.emulator.hack.ppu.hdNoSpriteLimit);
  emulator->configure("Hacks/PPU/HD-TrueColor", settings.emulator.hack.ppu.hdTrueColor);
  emulator->configure("Hacks/PPU/NoVRAMBlocking", settings.emulator.hack.ppu.noVRAMBlocking);
  emulator->configure("Hacks/PPU/Mode7/Scale", settings.emulator.hack.ppu.mode7.scale);
  emulator->configure("Hacks/PPU/Mode7/Perspective", settings.emulator.hack.ppu.mode7.perspective);
  emulator->configure("Hacks/PPU/Mode7/Supersample", settings.emulator.hack.ppu.mode7.supersample);
  emulator->configure("Hacks/PPU/Mode7/Mosaic", settings.emulator.hack.ppu.mode7.mosaic);
  emulator->configure("Hacks/PPU/HDMode7/Scale", settings.emulator.hack.ppu.hdMode7.scale);
  emulator->configure("Hacks/PPU/HDMode7/Perspective", settings.emulator.hack.ppu.hdMode7.perspective);
  emulator->configure("Hacks/PPU/HDMode7/Supersample", settings.emulator.hack.ppu.hdMode7.supersample);
  emulator->configure("Hacks/PPU/HDMode7/SsFactor", settings.emulator.hack.ppu.hdMode7.ssFactor);
  emulator->configure("Hacks/PPU/HDMode7/Mosaic", settings.emulator.hack.ppu.hdMode7.mosaic);
  emulator->configure("Hacks/PPU/HDMode7/GpuSupersample", settings.emulator.hack.ppu.hdMode7.gpuSupersample);
  emulator->configure("Hacks/PPU/HDMode7/WsMode", settings.emulator.hack.ppu.hdMode7.wsMode);
  emulator->configure("Hacks/PPU/HDMode7/Widescreen", settings.emulator.hack.ppu.hdMode7.widescreen);
  emulator->configure("Hacks/DSP/Fast", settings.emulator.hack.dsp.fast);
  emulator->configure("Hacks/DSP/Cubic", settings.emulator.hack.dsp.cubic);
  emulator->configure("Hacks/DSP/EchoShadow", settings.emulator.hack.dsp.echoShadow);
  emulator->configure("Hacks/Coprocessor/DelayedSync", settings.emulator.hack.coprocessor.delayedSync);
  emulator->configure("Hacks/Coprocessor/PreferHLE", settings.emulator.hack.coprocessor.preferHLE);
  emulator->configure("Hacks/SuperFX/Overclock", settings.emulator.hack.superfx.overclock);
  if(!emulator->load()) return;

  gameQueue = {};
  screenshot = {};
  if(!verified() && emulatorSettings.warnOnUnverifiedGames.checked()) {
    //Emulator::loaded() is true at this point:
    //prevent Program::main() from calling Emulator::run() during this dialog window
    auto lock = acquire();
    auto response = MessageDialog(
      "警告：此游戏镜像未经验证。\n"
      "运行它“可能”存在安全风险。\n\n"
      "你仍然希望运行这个游戏吗？"
    ).setAlignment(*presentation).question({"Always", "Yes", "No"});
    if(response == "No") {
      emulator->unload();
      return showMessage("游戏加载已取消");
    }
    if(response == "Always") {
      emulatorSettings.warnOnUnverifiedGames.setChecked(false).doToggle();
    }
  }
  hackCompatibility();
  emulator->power();
  ppuRendererActive = settings.emulator.hack.ppu.hd ? 2 : settings.emulator.hack.ppu.fast ? 1 : 0;
  enhancementSettings.ppuRendererChanged();
  if(emulatorSettings.autoLoadStateOnLoad.checked()) {
    program.loadState("Quick/Undo");
  }
  showMessage({
    verified() ? "Verified game loaded" : "Game loaded",
    appliedPatch() ? " and patch applied" : ""
  });
  presentation.setFocused();
  presentation.setTitle(emulator->title());
  presentation.resetSystem.setEnabled(true);
  presentation.unloadGame.setEnabled(true);
  presentation.toolsMenu.setVisible(true);
  presentation.netplayMenu.setVisible(true);
  presentation.updateStateMenus();
  presentation.speedNormal.setChecked();
  presentation.runEmulation.setChecked().doActivate();
  if(auto slot = getenv("BSNES_LOAD_STATE")) {
    program.loadState(slot);
  }
  presentation.updateProgramIcon();
  presentation.updateStatusIcon();
  rewindReset();  //starts rewind state recording
  movieMode(Movie::Mode::Inactive);  //to set initial movie menu state
  cheatFinder.restart();  //clear any old cheat search results
  cheatEditor.loadCheats();
  stateManager.loadStates();
  manifestViewer.loadManifest();

  string games;
  if(auto& game = superFamicom) games.append(game.option, ";", game.location, "|");
  if(auto& game = gameBoy     ) games.append(game.option, ";", game.location, "|");
  if(auto& game = bsMemory    ) games.append(game.option, ";", game.location, "|");
  if(auto& game = sufamiTurboA) games.append(game.option, ";", game.location, "|");
  if(auto& game = sufamiTurboB) games.append(game.option, ";", game.location, "|");
  presentation.addRecentGame(games.trimRight("|", 1L));

  updateVideoPalette();
  updateVideoEffects();
  updateAudioEffects();
  updateAudioFrequency();
}

auto Program::loadFile(string location) -> vector<uint8_t> {
  if(Location::suffix(location).downcase() == ".zip") {
    Decode::ZIP archive;
    if(archive.open(location)) {
      for(auto& file : archive.file) {
        auto type = Location::suffix(file.name).downcase();
        if(type == ".sfc" || type == ".smc" || type == ".gb" || type == ".gbc" || type == ".bs" || type == ".st") {
          return archive.extract(file);
        }
      }
    }
    return {};
  } else if(Location::suffix(location).downcase() == ".7z") {
    return LZMA::extract(location);
  } else {
    return file::read(location);
  }
}

auto Program::loadSuperFamicom(string location) -> bool {
  string manifest;
  vector<uint8_t> rom;

  if(location.endsWith("/")) {
    manifest = file::read({location, "manifest.bml"});
    rom.append(file::read({location, "program.rom"}));
    rom.append(file::read({location, "data.rom"}));
    rom.append(file::read({location, "expansion.rom"}));
    for(auto& filename : directory::files(location, "*.boot.rom"   )) rom.append(file::read({location, filename}));
    for(auto& filename : directory::files(location, "*.program.rom")) rom.append(file::read({location, filename}));
    for(auto& filename : directory::files(location, "*.data.rom"   )) rom.append(file::read({location, filename}));
  } else {
    manifest = file::read({Location::notsuffix(location), ".bml"});
    rom = loadFile(location);
  }
  if(rom.size() < 0x8000) return false;

  if((rom.size() & 0x7fff) == 512) {
    //remove copier header
    memory::move(&rom[0], &rom[512], rom.size() - 512);
    rom.resize(rom.size() - 512);
  }

  if(!superFamicom.patched) superFamicom.patched = applyPatchIPS(rom, location);
  if(!superFamicom.patched) superFamicom.patched = applyPatchBPS(rom, location);
  auto heuristics = Heuristics::SuperFamicom(rom, location);
  auto sha256 = Hash::SHA256(rom).digest();
  superFamicom.title = heuristics.title();
  superFamicom.region = heuristics.videoRegion();
  if(auto document = BML::unserialize(string::read(locate("Database/Super Famicom.bml")))) {
    if(auto game = document[{"game(sha256=", sha256, ")"}]) {
      manifest = BML::serialize(game);
      //the internal ROM header title is not present in the database, but is needed for internal core overrides
      manifest.append("  title: ", superFamicom.title, "\n");
      superFamicom.verified = true;
    }
  }
  superFamicom.manifest = manifest ? manifest : heuristics.manifest();
  hackPatchMemory(rom);
  superFamicom.document = BML::unserialize(superFamicom.manifest);
  superFamicom.location = location;

  uint offset = 0;
  if(auto size = heuristics.programRomSize()) {
    superFamicom.program.resize(size);
    memory::copy(&superFamicom.program[0], &rom[offset], size);
    offset += size;
  }
  if(auto size = heuristics.dataRomSize()) {
    superFamicom.data.resize(size);
    memory::copy(&superFamicom.data[0], &rom[offset], size);
    offset += size;
  }
  if(auto size = heuristics.expansionRomSize()) {
    superFamicom.expansion.resize(size);
    memory::copy(&superFamicom.expansion[0], &rom[offset], size);
    offset += size;
  }
  if(auto size = heuristics.firmwareRomSize()) {
    superFamicom.firmware.resize(size);
    memory::copy(&superFamicom.firmware[0], &rom[offset], size);
    offset += size;
  }

  return true;
}

auto Program::loadGameBoy(string location) -> bool {
  string manifest;
  vector<uint8_t> rom;

  if(location.endsWith("/")) {
    manifest = file::read({location, "manifest.bml"});
    rom.append(file::read({location, "program.rom"}));
  } else {
    manifest = file::read({Location::notsuffix(location), ".bml"});
    rom = loadFile(location);
  }
  if(rom.size() < 0x4000) return false;

  gameBoy.patched = applyPatchIPS(rom, location) || applyPatchBPS(rom, location);
  auto heuristics = Heuristics::GameBoy(rom, location);
  auto sha256 = Hash::SHA256(rom).digest();
  if(auto document = BML::unserialize(string::read(locate("Database/Game Boy.bml")))) {
    if(auto game = document[{"game(sha256=", sha256, ")"}]) {
      manifest = BML::serialize(game);
      gameBoy.verified = true;
    }
  }
  if(auto document = BML::unserialize(string::read(locate("Database/Game Boy Color.bml")))) {
    if(auto game = document[{"game(sha256=", sha256, ")"}]) {
      manifest = BML::serialize(game);
      gameBoy.verified = true;
    }
  }
  gameBoy.manifest = manifest ? manifest : heuristics.manifest();
  gameBoy.document = BML::unserialize(gameBoy.manifest);
  gameBoy.location = location;

  gameBoy.program = rom;
  return true;
}

auto Program::loadBSMemory(string location) -> bool {
  string manifest;
  vector<uint8_t> rom;

  if(location.endsWith("/")) {
    manifest = file::read({location, "manifest.bml"});
    rom.append(file::read({location, "program.rom"}));
    rom.append(file::read({location, "program.flash"}));
  } else {
    manifest = file::read({Location::notsuffix(location), ".bml"});
    rom = loadFile(location);
  }
  if(rom.size() < 0x8000) return false;

  bsMemory.patched = applyPatchIPS(rom, location) || applyPatchBPS(rom, location);
  auto heuristics = Heuristics::BSMemory(rom, location);
  auto sha256 = Hash::SHA256(rom).digest();
  if(auto document = BML::unserialize(string::read(locate("Database/BS Memory.bml")))) {
    if(auto game = document[{"game(sha256=", sha256, ")"}]) {
      manifest = BML::serialize(game);
      bsMemory.verified = true;
    }
  }
  bsMemory.manifest = manifest ? manifest : heuristics.manifest();
  bsMemory.document = BML::unserialize(bsMemory.manifest);
  bsMemory.location = location;

  bsMemory.program = rom;
  return true;
}

auto Program::loadSufamiTurboA(string location) -> bool {
  string manifest;
  vector<uint8_t> rom;

  if(location.endsWith("/")) {
    manifest = file::read({location, "manifest.bml"});
    rom.append(file::read({location, "program.rom"}));
  } else {
    manifest = file::read({Location::notsuffix(location), ".bml"});
    rom = loadFile(location);
  }
  if(rom.size() < 0x20000) return false;

  sufamiTurboA.patched = applyPatchIPS(rom, location) || applyPatchBPS(rom, location);
  auto heuristics = Heuristics::SufamiTurbo(rom, location);
  auto sha256 = Hash::SHA256(rom).digest();
  if(auto document = BML::unserialize(string::read(locate("Database/Sufami Turbo.bml")))) {
    if(auto game = document[{"game(sha256=", sha256, ")"}]) {
      manifest = BML::serialize(game);
      sufamiTurboA.verified = true;
    }
  }
  sufamiTurboA.manifest = manifest ? manifest : heuristics.manifest();
  sufamiTurboA.document = BML::unserialize(sufamiTurboA.manifest);
  sufamiTurboA.location = location;

  sufamiTurboA.program = rom;
  return true;
}

auto Program::loadSufamiTurboB(string location) -> bool {
  string manifest;
  vector<uint8_t> rom;

  if(location.endsWith("/")) {
    manifest = file::read({location, "manifest.bml"});
    rom.append(file::read({location, "program.rom"}));
  } else {
    manifest = file::read({Location::notsuffix(location), ".bml"});
    rom = loadFile(location);
  }
  if(rom.size() < 0x20000) return false;

  sufamiTurboB.patched = applyPatchIPS(rom, location) || applyPatchBPS(rom, location);
  auto heuristics = Heuristics::SufamiTurbo(rom, location);
  auto sha256 = Hash::SHA256(rom).digest();
  if(auto document = BML::unserialize(string::read(locate("Database/Sufami Turbo.bml")))) {
    if(auto game = document[{"game(sha256=", sha256, ")"}]) {
      manifest = BML::serialize(game);
      sufamiTurboB.verified = true;
    }
  }
  sufamiTurboB.manifest = manifest ? manifest : heuristics.manifest();
  sufamiTurboB.document = BML::unserialize(sufamiTurboB.manifest);
  sufamiTurboB.location = location;

  sufamiTurboB.program = rom;
  return true;
}

auto Program::save() -> void {
  if(!emulator->loaded()) return;
  emulator->save();
}

auto Program::reset() -> void {
  if(!emulator->loaded()) return;
  rewindReset();  //don't allow rewinding past a reset point
  hackCompatibility();
  emulator->reset();
  showMessage("Game reset");
}

auto Program::power() -> void {
  if(!emulator->loaded()) return;
  rewindReset();
  hackCompatibility();
  emulator->power();
  showMessage("Power cycle");
}

auto Program::videoSupportsHdGpu() const -> bool {
  return video.driver() == "OpenGL 3.2";
}

auto Program::selectFastPpuDueToDriver(Window parent) -> void {
  MessageDialog({
    "错误: 该显示驱动 ", video.driver(), " 可能不支持 HD PPU。\n"
    "GPU 采样需要使用 OpenGL 3.2。\n"
    "建议在不加载游戏时进行设置。已改用 Fast PPU。"
  }).setAlignment(parent).error();

  settings.emulator.hack.ppu.hd = false;
  settings.emulator.hack.ppu.fast = true;
  settings.save();

  enhancementSettings.loadingMode7 = true;
  enhancementSettings.ppuRenderer.item(1).setSelected();
  enhancementSettings.loadingMode7 = false;
  enhancementSettings.reloadMode7Widgets();

  if(emulator->loaded() && ppuRendererActive == 2) {
    serializer s = emulator->serialize();
    hackCompatibility();
    bool restored = emulator->unserialize(s);
    if(!restored) {
      hackCompatibility();
      emulator->power();
    }
    ppuRendererActive = 1;
    rewindReset();
    showMessage(restored ? "PPU渲染器已应用" : "已应用PPU渲染器（游戏已重启）");
  } else {
    ppuRendererActive = 1;
    emulator->configure("Hacks/PPU/Fast", true);
    emulator->configure("Hacks/PPU/HD", false);
  }
  enhancementSettings.ppuRendererChanged();
}

auto Program::applyPPURenderer(Window parent) -> void {
  uint selected = 1;
  if(auto item = enhancementSettings.ppuRenderer.selected()) selected = item.offset();

  if(selected == 2 && settings.emulator.hack.ppu.hdMode7.gpuSupersample && !videoSupportsHdGpu()) {
    MessageDialog({
      "错误: 该显示驱动 ", video.driver(), " 不支持 HD PPU.。\n"
      "GPU 采样需要使用 OpenGL 3.2。 \n"
      "建议在不加载游戏时进行设置。未应用 HD。"
    }).setAlignment(parent).error();
    enhancementSettings.loadingMode7 = true;
    enhancementSettings.ppuRenderer.item(ppuRendererActive).setSelected();
    enhancementSettings.loadingMode7 = false;
    enhancementSettings.ppuRendererChanged();
    return;
  }

  settings.emulator.hack.ppu.fast = selected == 1;
  settings.emulator.hack.ppu.hd = selected == 2;

  if(!emulator->loaded()) {
    emulator->configure("Hacks/PPU/Fast", settings.emulator.hack.ppu.fast);
    emulator->configure("Hacks/PPU/HD", settings.emulator.hack.ppu.hd);
    settings.save();
    ppuRendererActive = selected;
    enhancementSettings.ppuRendererChanged();
    return;
  }

  bool oldScanline = ppuRendererActive == 1 || ppuRendererActive == 2;
  bool newScanline = selected == 1 || selected == 2;
  string warning = {
    "警告：在游戏加载时更改 PPU 渲染器可能会导致 bsnes 崩溃。\n"
    "为了安全起见，强烈建议您关闭载游戏。\n"
  };
  if(!oldScanline || !newScanline) {
    warning.append("\n切换至或切换出精确渲染器时，无法保持当前位置。\n");
  }
  warning.append("您现在仍然想继续进行渲染器更改吗？");

  if(MessageDialog(warning).setAlignment(parent).question() != "Yes") return;

  save();
  saveUndoState();
  settings.general.crashed = true;
  settings.save();

  bool restored = false;
  if(oldScanline && newScanline) {
    serializer s = emulator->serialize();
    hackCompatibility();
    restored = emulator->unserialize(s);
  }

  if(!restored) {
    hackCompatibility();
    emulator->power();
  }

  settings.general.crashed = false;
  settings.save();
  ppuRendererActive = selected;
  rewindReset();
  enhancementSettings.ppuRendererChanged();
  showMessage(restored ? "PPU渲染器已应用" : "已应用PPU渲染器（游戏已重启）");
}

auto Program::unload() -> void {
  if(!emulator->loaded()) return;
  //todo: video.clear() is not working on macOS/OpenGL 3.2
  if(auto [output, length] = video.acquire(1, 1); output) {
    *output = 0;
    video.release();
    video.output();
  }
  audio.clear();
  rewindReset();  //free up memory that is no longer needed
  movieStop();  //in case a movie is currently being played or recorded
  cheatEditor.saveCheats();
  toolsWindow.setVisible(false);
  if(emulatorSettings.autoSaveStateOnUnload.checked()) {
    saveUndoState();
  }
  emulator->unload();
  showMessage("Game unloaded");
  superFamicom = {};
  gameBoy = {};
  bsMemory = {};
  sufamiTurboA = {};
  sufamiTurboB = {};
  presentation.setTitle({"bsnes v", Emulator::Version, " | HD + netplay v", Emulator::NetplayVersion});
  presentation.resetSystem.setEnabled(false);
  presentation.unloadGame.setEnabled(false);
  presentation.toolsMenu.setVisible(false);
  presentation.netplayMenu.setVisible(false);
  presentation.updateProgramIcon();
  presentation.updateStatusIcon();
}

//a game is considered verified if the game plus its slot(s) are found in the games database
auto Program::verified() const -> bool {
  if(!emulator->loaded()) return false;
  if(superFamicom && !superFamicom.verified) return false;
  if(gameBoy && !gameBoy.verified) return false;
  if(bsMemory && !bsMemory.verified) return false;
  if(sufamiTurboA && !sufamiTurboA.verified) return false;
  if(sufamiTurboB && !sufamiTurboB.verified) return false;
  return true;
}
