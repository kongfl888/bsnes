auto PathSettings::create() -> void {
  setCollapsible();
  setVisible(false);

  layout.setSize({4, 6});
  layout.column(0).setAlignment(1.0);

  gamesLabel.setText("游戏：");
  gamesPath.setEditable(false);
  gamesAssign.setText("指定...").onActivate([&] {
    if(auto location = program.selectPath()) {
      settings.path.games = location;
      refreshPaths();
    }
  });
  gamesReset.setText("重置").onActivate([&] {
    settings.path.games = "";
    refreshPaths();
  });

  patchesLabel.setText("补丁：");
  patchesPath.setEditable(false);
  patchesAssign.setText("指定...").onActivate([&] {
    if(auto location = program.selectPath()) {
      settings.path.patches = location;
      refreshPaths();
    }
  });
  patchesReset.setText("重置").onActivate([&] {
    settings.path.patches = "";
    refreshPaths();
  });

  savesLabel.setText("存档：");
  savesPath.setEditable(false);
  savesAssign.setText("指定...").onActivate([&] {
    if(auto location = program.selectPath()) {
      settings.path.saves = location;
      refreshPaths();
    }
  });
  savesReset.setText("重置").onActivate([&] {
    settings.path.saves = "";
    refreshPaths();
  });

  cheatsLabel.setText("金手指：");
  cheatsPath.setEditable(false);
  cheatsAssign.setText("指定...").onActivate([&] {
    if(auto location = program.selectPath()) {
      settings.path.cheats = location;
      refreshPaths();
    }
  });
  cheatsReset.setText("重置").onActivate([&] {
    settings.path.cheats = "";
    refreshPaths();
  });

  statesLabel.setText("即时存档：");
  statesPath.setEditable(false);
  statesAssign.setText("指定...").onActivate([&] {
    if(auto location = program.selectPath()) {
      settings.path.states = location;
      refreshPaths();
    }
  });
  statesReset.setText("重置").onActivate([&] {
    settings.path.states = "";
    refreshPaths();
  });

  screenshotsLabel.setText("截图：");
  screenshotsPath.setEditable(false);
  screenshotsAssign.setText("指定...").onActivate([&] {
    if(auto location = program.selectPath()) {
      settings.path.screenshots = location;
      refreshPaths();
    }
  });
  screenshotsReset.setText("重置").onActivate([&] {
    settings.path.screenshots = "";
    refreshPaths();
  });

  refreshPaths();
}

auto PathSettings::refreshPaths() -> void {
  if(auto location = settings.path.games) {
    gamesPath.setText(location).setForegroundColor();
  } else {
    gamesPath.setText("（最近使用）").setForegroundColor({128, 128, 128});
  }
  if(auto location = settings.path.patches) {
    patchesPath.setText(location).setForegroundColor();
  } else {
    patchesPath.setText("（和游戏同目录）").setForegroundColor({128, 128, 128});
  }
  if(auto location = settings.path.saves) {
    savesPath.setText(location).setForegroundColor();
  } else {
    savesPath.setText("（和游戏同目录）").setForegroundColor({128, 128, 128});
  }
  if(auto location = settings.path.cheats) {
    cheatsPath.setText(location).setForegroundColor();
  } else {
    cheatsPath.setText("（和游戏同目录）").setForegroundColor({128, 128, 128});
  }
  if(auto location = settings.path.states) {
    statesPath.setText(location).setForegroundColor();
  } else {
    statesPath.setText("（和游戏同目录）").setForegroundColor({128, 128, 128});
  }
  if(auto location = settings.path.screenshots) {
    screenshotsPath.setText(location).setForegroundColor();
  } else {
    screenshotsPath.setText("（和游戏同目录）").setForegroundColor({128, 128, 128});
  }
}
