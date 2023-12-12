<template>
    <el-container style="width: 1100px">
        <el-header style="margin-top: 20px">
            <el-row>
                <el-col :span="8">
                    <el-button type="primary" style="width: 300px;height: 50px" @click="inputGrammar">输入文法的源文件
                    </el-button>
                </el-col>
                <el-col :span="8">
                    <el-button type="primary" style="width: 300px;height: 50px" @click="run">启动</el-button>
                </el-col>
                <el-col :span="8">
                    <el-button type="primary" style="width: 300px;height: 50px" @click="saveGrammar">另存文法到文件
                    </el-button>
                </el-col>
            </el-row>
        </el-header>
        <el-main>
            <span>输入文法规则: </span>
            <br><br>
            <el-card style="width: 100%;">
                <el-input v-model="data" placeholder="请输入文法规则" type="textarea"
                          :autosize="{ minRows: 4, maxRows: 15 }"></el-input>
            </el-card>
            <br>
            <span>原始json数据(便于debug): </span>
            <br><br>
            <el-card style="width: 100%;max-height: 500px;overflow: auto;">
                <vue-json-pretty :highlightMouseoverNode="true"
                                 :collapsedOnClickBrackets="true"
                                 path="res" :data="pureData">
                </vue-json-pretty>
            </el-card>
            <br>
            <span>First集合和Follow集合: </span>
            <br><br>
            <el-card style="width: 100%;">
                <el-table
                        :data="data1"
                        stripe
                        border
                        style="width: 100%">
                    <el-table-column
                            prop="char"
                            label="字符"
                            width="80">
                    </el-table-column>
                    <el-table-column
                            prop="firstSet"
                            label="first集合"
                            width="450">
                    </el-table-column>
                    <el-table-column
                            prop="followSet"
                            label="follow集合"
                            width="450">
                    </el-table-column>
                </el-table>
            </el-card>
            <br>
            <span>LR(0)的DFA图: </span>
            <br><br>
            <el-card style="width: 100%;">
                <div id="myGraph" style="height: 500px"></div>
            </el-card>
            <br>
            <span>SLR(1)分析表: </span>
            <br><br>
            <div v-show="!isSlr1">
                <el-row>
                    <el-tag>
                        <span style="color: red">{{ errMessage }}</span>
                    </el-tag>
                </el-row>
                <br>
            </div>
            <el-card style="width: 100%;">
                <el-table
                        :data="data3"
                        stripe
                        border
                        style="width: 100%">
                    <el-table-column
                            label="状态"
                            type="index"
                            :index="indexMethod"
                            width="80">
                    </el-table-column>
                    <el-table-column
                            label="输入"
                            width="450">
                        <template v-for="(item) in slrTableHead1">
                            <el-table-column :prop="item" :label="item"></el-table-column>
                        </template>
                    </el-table-column>
                    <el-table-column
                            label="Goto"
                            width="450">
                        <template v-for="(item) in slrTableHead2">
                            <el-table-column :prop="item" :label="item"></el-table-column>
                        </template>
                    </el-table-column>
                </el-table>
            </el-card>
        </el-main>
    </el-container>
</template>

<script setup>
import VueJsonPretty from 'vue-json-pretty';
import {onMounted, ref} from "vue";
import {Network} from "vis-network";
import {DataSet} from "vis-data/peer/umd/vis-data.min";
import {ElMessage} from "element-plus";

let data = ref(''); // 用户输入的文法
let pureData = ref(null); // 文法分析后得到的原始json数据
let data1 = ref(null); // first和follow集合
let data2 = ref(null); // lr0的dfa图，但是后面其实没用到
let data3 = ref(null); // slr1分析表
let slrTableHead1 = ref([]); // action表头
let slrTableHead2 = ref([]); // goto表头
let isSlr1 = ref(true); // 当前输入的文法是否满足slr1文法的规则
let errMessage = ref(''); // 如果不满足slr1文法的规则就给出报错信息

async function inputGrammar() {
    data.value = await window.my.inputGrammar();
    if (data.value != null) {
        ElMessage({
            message: '输入文法成功!',
            type: 'success',
        })
    } else {
        ElMessage({
            message: '取消输入操作',
            type: 'warning',
        })
    }
}

async function run() {
    let r = await window.my.run(data.value);
    let result = JSON.parse(r);
    pureData.value = result;
    ElMessage({
        message: '启动分析成功!',
        type: 'success',
    })
    data1.value = result["firstAndFollowSet"];
    let nodeList = result["lr0"]["node"];
    let newNodeList = [];
    for (let i = 0; i < nodeList.length; ++i) {
        let text = "";
        for (let j = 0; j < nodeList[i].length; ++j) {
            text += nodeList[i][j].left + "->" + nodeList[i][j].right + "\n";
        }
        text += "状态" + i;
        let node = {
            id: i,
            label: text,
            shape: "box"
        }
        newNodeList.push(node);
    }
    let edgeList = result["lr0"]["edge"];
    let newEdgeList = [];
    // 给数组对象添加新的属性信息
    edgeList.map((item, index) => {
        newEdgeList.push(Object.assign({}, item, {"arrows": "to"}));
    });
    let visNodeList = new DataSet(newNodeList);
    let visEdgeList = new DataSet(newEdgeList);
    let container = document.getElementById("myGraph");
    let visData = {
        nodes: visNodeList,
        edges: visEdgeList,
    };
    let options = {};
    let network = new Network(container, visData, options);

    isSlr1.value = result["isSlr1"];
    if (isSlr1.value) {
        data3.value = result["slr1"];
        let tableHead = Object.keys(data3.value[0]);
        let reg = /[A-Z]/;
        slrTableHead1.value = [];
        slrTableHead2.value = [];
        for (let i = 0; i < tableHead.length; ++i) {
            // 非终结符
            if (reg.test(tableHead[i])) {
                slrTableHead2.value.push(tableHead[i]);
            } else {
                slrTableHead1.value.push(tableHead[i]);
            }
        }
    } else {
        errMessage.value = result["errResult"];
        ElMessage({
            message: '注意：当前文法不是SLR(1)文法!',
            type: 'warning',
        })
    }
}

async function saveGrammar() {
    let b = await window.my.saveGrammar(data.value);
    if (b) {
        ElMessage({
            message: '保存文法成功!',
            type: 'success',
        })
    } else {
        ElMessage({
            message: '取消保存操作',
            type: 'warning',
        })
    }
}

/**
 * 让表格的下标从0开始
 * @param index
 * @returns {*}
 */
function indexMethod(index) {
    return index;
}

onMounted(() => {
    console.log("onMounted");
})
</script>

<style scoped>
* {
    margin: 0;
    padding: 0;
}
</style>