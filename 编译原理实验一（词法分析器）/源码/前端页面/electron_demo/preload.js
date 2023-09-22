const {contextBridge, ipcRenderer} = require('electron');

contextBridge.exposeInMainWorld('my', {
    openFile: () => ipcRenderer.invoke('openFile'),
    runCode: (filename, target) => ipcRenderer.invoke('runCode', filename, target)
})
