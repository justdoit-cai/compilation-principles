const {contextBridge, ipcRenderer} = require('electron');

contextBridge.exposeInMainWorld('my', {
    run: (input) => ipcRenderer.invoke('run', input),
    inputGrammar: () => ipcRenderer.invoke('inputGrammar'),
    saveGrammar: (input) => ipcRenderer.invoke('saveGrammar', input),
})