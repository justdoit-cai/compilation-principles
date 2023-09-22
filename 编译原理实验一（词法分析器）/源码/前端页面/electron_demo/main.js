const {app, BrowserWindow, dialog, ipcMain, Menu} = require('electron');
const util = require("util");
const exec = util.promisify(require("child_process").exec);
const path = require("path");
const fs = require("fs");

/*隐藏electron的菜单栏*/
Menu.setApplicationMenu(null)

const createWindow = () => {
    const win = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            // 导入预加载脚本
            preload: path.join(__dirname, 'preload.js'),
            sandbox: false
        }
    })

    win.loadFile('index.html');
    // win.webContents.openDevTools();
}

async function handleOpenFile() {
    const {canceled, filePaths} = await dialog.showOpenDialog({
        title: '选择输入',
        buttonLabel: '选择',
        filters: [
            {name: 'c', extensions: ['cpp', 'c']},
            {name: 'random file', extensions: ['*']},
        ],
    });
    if (!canceled) {
        return filePaths[0];
    } else {
        return null;
    }
}

async function handleRunCode(event, filename, target) {
    let prefix = filename.split("\\");
    prefix.pop();
    let output = prefix.join("\\") + "\\" + target;

    try {
        fs.readFileSync("resources/extra/cLexer.exe");
    } catch (err) {
        var fileList = fs.readdirSync("./");
        return "ERROR: resources/extra/cLexer.exe不存在!\n当前所在文件夹为: " + __dirname + "\n当前目录下存在的文件有: "  + fileList;
    }
    let command = "cd ./resources/extra && cLexer.exe " + filename + " " + output;

    // 这种方式是异步的(exec本身就是异步的)，会导致exec还未结束就return了
    // child_process.exec(command, (error,stdout,stderr) => {
    //     if (error) {
    //         console.log("Command ERROR: \n", error);
    //         result = error;
    //     }
    //     console.log("Command Output: \n", stdout);
    //     console.log("Command Error Output: \n", stderr);
    //     result = stdout;
    // });
    /**
     * 注意下面这种方式是不带error的，并且这种方式必须要是下面的格式，不然打印出来的stdout,stderr是对象
     * const util = require("util");
     * const exec = util.promisify(require("child_process").exec);
     */
    const {stdout, stderr} = await exec(command);
    console.log("Command Output: \n", stdout);
    console.log("Command Error Output: \n", stderr);
    if (stderr) {
        return stderr;
    } else {
        return "success";
    }
}

app.whenReady().then(() => {
    ipcMain.handle('openFile', handleOpenFile);
    ipcMain.handle('runCode', handleRunCode);
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
