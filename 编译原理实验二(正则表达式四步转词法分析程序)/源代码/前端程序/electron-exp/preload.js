const {contextBridge, ipcRenderer} = require('electron');

contextBridge.exposeInMainWorld('my', {
    run: (input) => ipcRenderer.invoke('run', input),
})
