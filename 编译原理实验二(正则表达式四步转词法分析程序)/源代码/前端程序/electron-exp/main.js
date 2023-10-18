const {app, BrowserWindow, dialog, ipcMain, Menu} = require('electron');
const util = require("util");
const exec = util.promisify(require("child_process").exec);
const path = require("path");
const fs = require("fs");

/*隐藏electron的菜单栏*/
Menu.setApplicationMenu(null)

const createWindow = () => {
    const win = new BrowserWindow({
        width: 1200,
        height: 900,
        webPreferences: {
            // 导入预加载脚本
            preload: path.join(__dirname, 'preload.js'),
            sandbox: false
        }
    })

    win.loadFile('index.html');
    // win.webContents.openDevTools();
}

async function runLex(event, input) {
    let command = "cd ./resources/extra && lexicalAnalyzer.exe \"" + input + "\"";
    const {stdout, stderr} = await exec(command);
    // console.log("Command Output: \n", stdout);
    // console.log("Command Error Output: \n", stderr);
    if (stderr) {
        return stderr;
    } else {
        return stdout;
    }
}

app.whenReady().then(() => {
    ipcMain.handle('run', runLex);
    createWindow();
    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0) {
            createWindow();
        }
    })
})

// 适配mac
app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit()
    }
})
