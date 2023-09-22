let input = document.getElementById('input');
let output = document.getElementById('output');
let transform = document.getElementById('transform');
let hint = document.getElementById('hint');

let filename;

input.addEventListener('click', async () => {
    filename = await window.my.openFile();
    if (filename != null) {
        hint.innerText = filename;
    } else {
        hint.innerText = '未选择文件';
    }
})

transform.addEventListener('click', async () => {
    console.log(filename);
    console.log(output.value);
    if (filename === undefined) {
        alert('请先选择输入文件!');
        return;
    }
    let result;
    if (output.value === '') {
        console.log('a.out')
        result = await window.my.runCode(filename, 'a.out');
    } else {
        console.log('else')
        result = await window.my.runCode(filename, output.value);
    }
    alert(result);
})