const {contextBridge, ipcRenderer} = require('electron');

contextBridge.exposeInMainWorld('my', {
    run: () => ipcRenderer.invoke('run'),
    input: () => ipcRenderer.invoke('input'),
    output: () => ipcRenderer.invoke('output'),
})