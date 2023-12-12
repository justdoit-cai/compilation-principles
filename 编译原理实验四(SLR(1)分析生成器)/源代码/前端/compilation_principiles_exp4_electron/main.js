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
    // 开发测试时
    // win.loadURL("http://localhost:5173/");
    // win.webContents.openDevTools();
    // 打包生产时
    win.loadFile('index.html');
}

async function run(event, input) {
    const base64Encode = (data) => {
        const buffer = Buffer.from(data);
        return buffer.toString('base64');
    };
    // 由于输入的文法存在换行符，因此会无法成功传入到命令行执行，所以这里需要base64编码一下，将其化为一行
    const encodedString = base64Encode(input);
    // 打包生产时用的路径
    // let command = "cd ./resources/extra && compilationPrinciplesExp4.exe \"" + encodedString + "\"";
    // 开发测试时用的路径
    let command = "cd ./extra && compilationPrinciplesExp4.exe \"" + encodedString + "\"";
    const {stdout, stderr} = await exec(command);
    if (stderr) {
        console.log(stderr)
        return stderr;
    } else {
        console.log(stdout)
        return stdout;
    }
}

async function inputGrammar() {
    const {canceled, filePaths} = await dialog.showOpenDialog({
        title: '选择输入',
        buttonLabel: '选择',
        filters: [
            {name: 'file', extensions: ['*']},
        ],
    });
    if (!canceled) {
        let sourceFile = filePaths[0].toString();
        return fs.readFileSync(sourceFile).toString();
    } else {
        return null;
    }
}

async function saveGrammar(event, input) {
    const {canceled, filePaths} = await dialog.showOpenDialog({
        title: '选择输入',
        buttonLabel: '选择',
        filters: [
            {name: 'file', extensions: ['*']},
        ],
    });
    if (!canceled) {
        fs.writeFileSync(filePaths[0], input);
        return true;
    } else {
        return false;
    }
}
app.whenReady().then(() => {
    // 注册方法
    ipcMain.handle('run', run);
    ipcMain.handle('inputGrammar', inputGrammar);
    ipcMain.handle('saveGrammar', saveGrammar);
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