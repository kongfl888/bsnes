auto NetplayWindow::isValidIP(const string& ip) -> bool {
    auto parts = ip.split(".");
    if(parts.size() != 4) return false;

    for(auto& part : parts) {
        if(!part) return false;
        uint num = part.natural();
        if(num > 255) return false;
        if(part != "0" && num == 0) return false;
    }
    return true;
}

auto NetplayWindow::isLoopbackIP(const string& ip) -> bool {
    auto parts = ip.split(".");
    return (parts.size() == 4 && parts[0].natural() == 127);
}

auto NetplayWindow::isValidPort(const string& port) -> bool {
    if(!port) return false;
    uint portNum = port.natural();
    return (portNum >= 1 && portNum <= 65535);
}

auto NetplayWindow::isValidRemoteIP(const string& ip) -> bool {
    return isValidIP(ip) && (devMode || !isLoopbackIP(ip));
}

auto NetplayWindow::setValidationColor(LineEdit& field, bool valid, bool hasText) -> void {
    field.setBackgroundColor(hasText && !valid ? Color{255, 200, 200} : Color{});
}

auto NetplayWindow::roleText(const PlayerEntry& entry) -> string {
    if(entry.isSpectator) return "Spectator";
    if(entry.playerNumber == 0) return "Player";
    return {"Player ", entry.playerNumber};
}

auto NetplayWindow::rowText(const PlayerEntry& entry) -> string {
    return {roleText(entry), " | ", entry.ip, " | ", entry.port};
}

auto NetplayWindow::rebuildList() -> void {
    remotePlayersList.reset();
    for(auto& entry : entries) {
        remotePlayersList.append(ListViewItem().setText(rowText(entry)));
    }
}

auto NetplayWindow::selectedIndex() -> maybe<uint> {
    auto item = remotePlayersList.selected();
    if(!item || item.offset() >= entries.size()) return nothing;
    return item.offset();
}

auto NetplayWindow::updateEditingState() -> void {
    auto index = selectedIndex();

    btnRemove.setEnabled((bool)index);
    editIPValue.setEnabled((bool)index);
    editPortValue.setEnabled((bool)index);

    if(index) {
        auto& entry = entries[index()];
        editingLabel.setText({"编辑 ", roleText(entry), ":"});
        editIPValue.setText(entry.ip);
        editPortValue.setText(entry.port);
    } else {
        editingLabel.setText("选中一项进行编辑");
        editIPValue.setText("");
        editPortValue.setText("");
    }
}

auto NetplayWindow::updateSelectedIP(const string& ip) -> void {
    auto index = selectedIndex();
    if(!index) return;

    auto& entry = entries[index()];
    entry.ip = ip;
    remotePlayersList.item(index()).setText(rowText(entry));
}

auto NetplayWindow::updateSelectedPort(const string& port) -> void {
    auto index = selectedIndex();
    if(!index) return;

    auto& entry = entries[index()];
    entry.port = port;
    remotePlayersList.item(index()).setText(rowText(entry));
}

auto NetplayWindow::sortPlayerList() -> void {
    entries.sort([](const PlayerEntry& a, const PlayerEntry& b) -> bool {
        if(a.isSpectator != b.isSpectator) return !a.isSpectator;
        if(!a.isSpectator) return a.playerNumber < b.playerNumber;
        return false;
    });

    rebuildList();
}

auto NetplayWindow::create() -> void {
    layout.setPadding(5_sx, 5_sx);

    roleLabel.setText("你的身份：").setFont(Font().setBold());
    rolePlayer1.setText("玩家1").onActivate([&] { updateRole(Role::Player1); });
    rolePlayer2.setText("玩家2").onActivate([&] { updateRole(Role::Player2); });
    rolePlayer3.setText("玩家3").onActivate([&] { updateRole(Role::Player3); });
    rolePlayer4.setText("玩家4").onActivate([&] { updateRole(Role::Player4); });
    rolePlayer5.setText("玩家5").onActivate([&] { updateRole(Role::Player5); });
    roleSpectator.setText("观众").onActivate([&] { updateRole(Role::Spectator); });

    roleSpacer.setColor({192, 192, 192});

    connectionLabel.setText("本机设置：").setFont(Font().setBold());

    portLabel.setText("本机端口：");
    portValue.setText("55435").onChange([&] {
        string port = portValue.text().strip();
        bool valid = isValidPort(port);
        setValidationColor(portValue, valid, (bool)port);
        if(valid) config.localPort = port.natural();
    });

    spectatorPlayerCountLabel.setText("当前会话玩家数：");
    spectatorPlayerCountValue.setText("2").onChange([&] {
        config.spectatorPlayerCount = spectatorPlayerCountValue.text().strip().integer();
    });

    rollbackLabel.setText("回滚帧数：");
    rollbackValue.setText("8").setAlignment(0.5);
    rollbackSlider.setLength(13).setPosition(8).onChange([&] {
        uint frames = rollbackSlider.position();
        rollbackValue.setText({frames});
        config.rollbackframes = frames;
    });

    delayLabel.setText("输入延迟：");
    delayValue.setText("2").setAlignment(0.5);
    delaySlider.setLength(11).setPosition(2).onChange([&] {
        uint delay = delaySlider.position();
        delayValue.setText({delay});
        config.localDelay = delay;
    });

    sliderSpacer.setColor({192, 192, 192});

    remotePlayersLabel.setText("其他玩家：").setFont(Font().setBold());
    remotePlayersList.onChange([&] { updateEditingState(); });

    editIPLabel.setText("IP：");
    editIPValue.setText("").setEnabled(false).onChange([&] {
        string ip = editIPValue.text().strip();
        bool valid = isValidRemoteIP(ip);
        setValidationColor(editIPValue, valid, (bool)ip);
        if(valid) updateSelectedIP(ip);
    });

    editPortLabel.setText("端口：");
    editPortValue.setText("").setEnabled(false).onChange([&] {
        string port = editPortValue.text().strip();
        bool valid = isValidPort(port);

        setValidationColor(editPortValue, valid, (bool)port);
        if(valid) updateSelectedPort(port);
    });

    btnAddPlayer.setText("添加玩家").onActivate([&] { addPlayer(); });
    btnAddSpectator.setText("添加观众").onActivate([&] { addSpectator(); });
    btnRemove.setText("移除").onActivate([&] { removeSelectedPlayer(); }).setEnabled(false);

    editingLabel.setFont(Font().setBold());

    rolePlayer1.setChecked();
    updateRole(Role::Player1);

    bottomSpacer.setColor({192, 192, 192});

    devModeCheck.setText("开发模式").setChecked(false).onToggle([&] {
        devMode = devModeCheck.checked();
        string ip = editIPValue.text().strip();
        if(ip) setValidationColor(editIPValue, isValidRemoteIP(ip), true);
    });

    desyncCheck.setText("不同步检测").setChecked(false).setToolTip(
        "每一帧将模拟器状态校验和与其他玩家比对。\n"
        "Desyncs are written to the desyncs folder alongside the states that caused them.\n\n"
        "Costs performance, so leave this off unless you are hunting a desync."
    ).onToggle([&] { config.desyncDetection = desyncCheck.checked(); });

    btnStart.setText("开始联机").setIcon(Icon::Media::Play).onActivate([&] { startSession(); });
    btnCancel.setText("取消").setIcon(Icon::Action::Quit).onActivate([&] { doClose(); });

    setTitle("联机设置");
    setSize({500_sx, 365_sy});
    setAlignment({0.5, 0.5});
    onClose([&] { setVisible(false); });
}

auto NetplayWindow::updateRole(Role role) -> void {
    currentRole = role;
    bool isSpectator = (role == Role::Spectator);

    if(isSpectator) {
        remotePlayersLabel.setText("要观战的玩家：");

        entries.reset();
        entries.append(PlayerEntry{false, 0, "127.0.0.1", "55435"});
        rebuildList();
        remotePlayersList.item(0).setSelected();

        btnAddPlayer.setEnabled(false);
        btnAddSpectator.setEnabled(false);

        spectatorPlayerCountLabel.setVisible(true);
        spectatorPlayerCountValue.setVisible(true);
        slidersLayout.setVisible(false);
    } else {
        remotePlayersLabel.setText("其他玩家：");
        autoPopulatePlayerList();

        btnAddPlayer.setEnabled(true);
        btnAddSpectator.setEnabled(true);

        spectatorPlayerCountLabel.setVisible(false);
        spectatorPlayerCountValue.setVisible(false);
        slidersLayout.setVisible(true);
    }

    updateEditingState();
}

auto NetplayWindow::autoPopulatePlayerList() -> void {
    entries.reset();
    uint myPlayerNum = roleToPlayerIndex(currentRole) + 1;
    uint maxPlayer = myPlayerNum < 2 ? 2 : myPlayerNum;

    for(uint p = 1; p <= maxPlayer; p++) {
        if(p != myPlayerNum) {
            entries.append(PlayerEntry{false, p, "127.0.0.1", "55435"});
        }
    }

    rebuildList();
}

auto NetplayWindow::addPlayer() -> void {
    uint myPlayerNum = roleToPlayerIndex(currentRole) + 1;
    vector<uint> usedPlayers{myPlayerNum};

    for(auto& entry : entries) {
        if(!entry.isSpectator && entry.playerNumber) usedPlayers.append(entry.playerNumber);
    }

    for(uint p = 1; p <= 5; p++) {
        if(!usedPlayers.find(p)) {
            entries.append(PlayerEntry{false, p, "127.0.0.1", "55435"});
            sortPlayerList();
            remotePlayersList.resizeColumn();
            updateEditingState();
            return;
        }
    }

    MessageDialog()
        .setTitle("玩家上限")
        .setText("全部5个玩家位已占用。\n\n你可以添加观众。")
        .information();
}

auto NetplayWindow::addSpectator() -> void {
    entries.append(PlayerEntry{true, 0, "127.0.0.1", "55435"});
    rebuildList();
    remotePlayersList.resizeColumn();
    updateEditingState();
}

auto NetplayWindow::removeSelectedPlayer() -> void {
    auto index = selectedIndex();
    if(!index) return;

    if(currentRole == Role::Spectator) {
        MessageDialog()
            .setTitle("无法移除")
            .setText("作为观众，你必须连接到一名玩家。\n\n"
                     "您可以编辑IP和端口，但无法删除此条目。")
            .information();
        return;
    }

    auto& entry = entries[index()];

    if(!entry.isSpectator) {
        uint myPlayerNum = roleToPlayerIndex(currentRole) + 1;

        if(entry.playerNumber < myPlayerNum) {
            MessageDialog()
                .setTitle("无法移除玩家")
                .setText({"你选中了玩家 ", myPlayerNum, ", 因此需要有 players 1-",
                         myPlayerNum - 1, " 已配置.\n\n",
                         "你只能移除编号比你高的玩家。"})
                .information();
            return;
        }

        uint playerCount = 1;
        for(auto& e : entries) {
            if(!e.isSpectator) playerCount++;
        }

        if(playerCount <= 2) {
            MessageDialog()
                .setTitle("最低玩家数量要求")
                .setText("联机会话至少需要2名玩家。\n\n"
                         "无法移除此玩家，因为这样将只剩下1名玩家。")
                .information();
            return;
        }
    }

    entries.remove(index());
    rebuildList();
    remotePlayersList.resizeColumn();
    updateEditingState();
}

auto NetplayWindow::startSession() -> void {
    if(!config.localPort || config.localPort > 65535) {
        MessageDialog()
            .setTitle("端口无效")
            .setText("请输入有效的端口号（1-65535）。")
            .information();
        return;
    }

    uint8 localPlayer = roleToPlayerIndex(currentRole);
    bool isSpectator = (localPlayer == 255);
    uint myPlayerNum = localPlayer + 1;

    if(isSpectator && (config.spectatorPlayerCount < 2 || config.spectatorPlayerCount > 5)) {
        MessageDialog()
            .setTitle("玩家数量无效")
            .setText("请指定会话内玩家数量（2-5）。")
            .information();
        return;
    }

    vector<string> remotes, spectators;

    for(auto& entry : entries) {
        if(!entry.ip || !entry.port) continue;

        if(!isValidRemoteIP(entry.ip)) {
            MessageDialog()
                .setTitle("IP地址无效")
                .setText({"联机不允许使用回环地址（127.x.x.x）。\n\n",
                         "玩家: ", roleText(entry), "\nIP: ", entry.ip, "\n\n",
                         "请输入远程玩家的实际网络IP地址。\n",
                         "或者启用开发者模式以允许使用环回地址。"})
                .information();
            return;
        }

        if(devMode && isLoopbackIP(entry.ip) && entry.port.natural() == config.localPort) {
            MessageDialog()
                .setTitle("配置无效")
                .setText({"玩家： ", roleText(entry), " is set to 127.0.0.1:", entry.port,
                         "，和你自己监听的地址和端口相同。\n\n",
                         "你自己的数据包会回传给你，从而错误地显示为连接。\n",
                         "为该条目使用不同的端口（例如：另一个实例自己的端口）。"})
                .information();
            return;
        }

        if(entry.isSpectator) {
            spectators.append({entry.ip, ":", entry.port});
        } else if(entry.playerNumber) {
            if(entry.playerNumber != myPlayerNum) {
                remotes.append({entry.ip, ":", entry.port});
            }
        } else {
            remotes.append({entry.ip, ":", entry.port});
        }
    }

    if(remotes.size() == 0) {
        MessageDialog()
            .setTitle("缺少连接信息")
            .setText(isSpectator
                ? "观战需要连接到一名玩家。\n\n输入会话中任意玩家的IP地址。"
                : "您需要至少添加一名其他玩家。\n\n每位玩家应输入所有其他玩家的IP地址。\n这是一种点对点连接——每个人都能连接到每个人。")
            .information();
        return;
    }

    vector<string> finalRemotes = remotes;
    if(isSpectator && config.spectatorPlayerCount > 1) {
        for(uint i = 0; i < config.spectatorPlayerCount - 1; i++) {
            finalRemotes.append("SPEC_FILLER");
        }
    }

    program.netplayStart(config.localPort, localPlayer, config.rollbackframes,
                        config.localDelay, finalRemotes, spectators, config.desyncDetection);
    doClose();
}

auto NetplayWindow::roleToPlayerIndex(Role role) -> uint8 {
    switch(role) {
        case Role::Player1: return 0;
        case Role::Player2: return 1;
        case Role::Player3: return 2;
        case Role::Player4: return 3;
        case Role::Player5: return 4;
        case Role::Spectator: return 255;
    }
    return 0;
}

auto NetplayWindow::setVisible(bool visible) -> NetplayWindow& {
    if(visible) {
        Application::processEvents();
    }
    return Window::setVisible(visible), *this;
}

auto NetplayWindow::show() -> void {
    setVisible();
    setFocused();
}
