<template>
    <el-container style="width: 1100px">
        <el-header style="margin-bottom: 10px">
            <el-row>
                <el-col :span="24">
                    <div>
                        <p style="padding-bottom: 10px; padding-top: 10px">请输入正则表达式: </p>
                        <el-row :gutter="40">
                            <el-col :span="18">
                                <el-input v-model="input" placeholder="Please input"/>
                            </el-col>
                            <el-col :span="6">
                                <el-button type="primary" style="width: 150px" @click="runLex">启动</el-button>
                            </el-col>
                        </el-row>
                    </div>
                </el-col>
            </el-row>
        </el-header>
        <el-main>
            <el-row :gutter="40">
                <el-col :span="6">
                    <el-button color="#626aef" :dark="isDark" style="width: 150px;" @click="handleNFA">
                        显示NFA
                    </el-button>
                </el-col>
                <el-col :span="6">
                    <el-button color="#626aef" :dark="isDark" style="width: 150px;" @click="handleDFA">
                        显示DFA
                    </el-button>
                </el-col>
                <el-col :span="6">
                    <el-button color="#626aef" :dark="isDark" style="width: 150px;" @click="handleSimplifiedDFA">
                        显示最小化DFA
                    </el-button>
                </el-col>
                <el-col :span="6">
                    <el-button color="#626aef" :dark="isDark" style="width: 150px;" @click="handleOutputCode">
                        显示词法分析程序
                    </el-button>
                </el-col>
            </el-row>
            <el-divider/>

            <div v-if="showCode">
                <el-card>
                    <pre><code v-text="code"></code></pre>
                </el-card>
            </div>
            <div v-show="showGraph">
                <el-row>
                    <span>{{ message }}</span>
                </el-row>
                <br>
                <div id="myGraph" style="width: 100%; height: 600px; border: 2px; background-color: #fff;"></div>
            </div>
        </el-main>
    </el-container>
</template>
<script setup>
import {onMounted, ref} from 'vue'
import {Network} from "vis-network";
import {DataSet} from "vis-data/peer/umd/vis-data.min";
import {ElMessage} from "element-plus";

const input = ref('');
let message = ref('');
let code = ref('');
let showCode = ref(false);
let showGraph = ref(true);
let data;

async function runLex() {
    if (input.value === '') {
        ElMessage({
            message: '请先输入正则字符串!',
            type: 'error',
        })
        return;
    }
    let ret = await window.my.run(input.value);
    data = JSON.parse(ret);
    ElMessage({
        message: '生成成功!点击下列按钮显示结果',
        type: 'success',
    })
}

function handleNFA() {
    showCode.value = false;
    showGraph.value = true;
    console.log("nfa");
    if (!data) {
        ElMessage({
            message: '请先启动!',
            type: 'error',
        })
    }
    let obj = data['inner1'];

    let start = obj['start'];
    let end = obj['end'];

    message.value = '起点下标: ' + start + ", 终点下标: " + end;

    let edgeList = obj['data'];

    let nodeList = [];
    for (let i = start; i <= end; ++i) {
        let node = {
            id: i,
            label: i.toString()
        }
        nodeList.push(node);
    }

    let newEdgeList = [];
    edgeList.map((item, index) => {
        newEdgeList.push(Object.assign({}, item, {"arrows": "to"}));
    });

    let visNodeList = new DataSet(nodeList);
    let visEdgeList = new DataSet(newEdgeList);

    // 获取容器
    var container = document.getElementById('myGraph');
    // 将数据赋值给vis 数据格式化器
    var visData = {
        nodes: visNodeList,
        edges: visEdgeList
    };
    var options = {};
    // 初始化关系图
    var network = new Network(container, visData, options);
}

function handleDFA() {
    showCode.value = false;
    showGraph.value = true;
    console.log("dfa");
    if (!data) {
        ElMessage({
            message: '请先启动!',
            type: 'error',
        })
    }
    let obj = data['inner2'];

    let start = obj['start'];
    let end = obj['end']; // DFA的end是数组

    message.value = '起点下标: ' + start + ", 终点下标: " + end;

    let edgeList = obj['data']["edgeList"];
    let stateList = obj['data']['nodeList'];



    let nodeList = [];
    for (let i = start; i <= end[end.length - 1]; ++i) {
        let label = i.toString() + ":{" + stateList[i] + "}";
        let node = {
            id: i,
            label: label
        }
        nodeList.push(node);
    }

    let newEdgeList = [];
    edgeList.map((item, index) => {
        newEdgeList.push(Object.assign({}, item, {"arrows": "to"}));
    });

    let visNodeList = new DataSet(nodeList);
    let visEdgeList = new DataSet(newEdgeList);

    // 获取容器
    var container = document.getElementById('myGraph');
    // 将数据赋值给vis 数据格式化器
    var visData = {
        nodes: visNodeList,
        edges: visEdgeList
    };
    var options = {};
    // 初始化关系图
    var network = new Network(container, visData, options);
}

function handleSimplifiedDFA() {
    showCode.value = false;
    showGraph.value = true;
    if (!data) {
        ElMessage({
            message: '请先启动!',
            type: 'error',
        })
    }
    let obj = data['inner3'];

    let start = obj['start'];
    let end = obj['end'];

    message.value = '起点下标: ' + start + ", 终点下标: " + end;

    let edgeList = obj['data'];

    let nodeList = [];
    let s = new Set();
    for (let i in edgeList) {
        s.add(edgeList[i].from);
        s.add(edgeList[i].to);
    }
    for (let i of s.values()) { // 注意这里set遍历的方式
        let node = {
            id: i,
            label: i.toString()
        }
        nodeList.push(node);
    }
    let newEdgeList = [];
    edgeList.map((item, index) => {
        newEdgeList.push(Object.assign({}, item, {"arrows": "to"}));
    });
    let visNodeList = new DataSet(nodeList);
    let visEdgeList = new DataSet(newEdgeList);

    // 获取容器
    var container = document.getElementById('myGraph');
    // 将数据赋值给vis 数据格式化器
    var visData = {
        nodes: visNodeList,
        edges: visEdgeList
    };
    var options = {};
    // 初始化关系图
    var network = new Network(container, visData, options);
}

function handleOutputCode() {
    if (!data) {
        ElMessage({
            message: '请先启动!',
            type: 'error',
        })
    }
    code = data["outputCode"];
    showCode.value = true;
    showGraph.value = false;
}
onMounted(() => {
    console.log("mounted");
})
</script>