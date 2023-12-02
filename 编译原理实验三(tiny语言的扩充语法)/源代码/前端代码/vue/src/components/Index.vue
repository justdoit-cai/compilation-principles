<template>
    <el-container style="width: 1100px;">
        <el-header style="margin-top: 20px">
            <el-row>
                <el-col :span="8">
                    <el-button type="primary" style="width: 300px;height: 50px" @click="input">选择源程序文件</el-button>
                </el-col>
                <el-col :span="8">
                    <el-button type="primary" style="width: 300px;height: 50px" @click="run">启动</el-button>
                </el-col>
                <el-col :span="8">
                    <el-button type="primary" style="width: 300px;height: 50px" @click="output">保存输出结果到文件</el-button>
                </el-col>
            </el-row>
        </el-header>

        <el-main>
            <div style="width: 510px;float: left;margin-right: 40px">
                <span>输入</span>
                <el-card style="height: 700px;overflow: auto">
                        <pre><code>{{ inputData }}</code></pre>
                </el-card>
            </div>
            <div style="width: 510px;float: left">
                <span>输出</span>
            <el-card style="height: 700px;overflow: auto;">
                <vue-json-pretty :highlightMouseoverNode="true"
                                 :collapsedOnClickBrackets="true"
                                 path="res" :data="outputData">
                </vue-json-pretty>
            </el-card>
            </div>
        </el-main>
    </el-container>
</template>

<script setup>
import VueJsonPretty from 'vue-json-pretty';
import 'vue-json-pretty/lib/styles.css';
import {ref} from "vue";
import {ElMessage} from "element-plus";
let inputData = ref('');
let outputData = ref({});
let data = ref({});


async function input() {
    inputData.value = await window.my.input();
    ElMessage({
        message: '输入成功!',
        type: 'success',
    })
}
async function run() {
    outputData.value = await window.my.run();
    outputData.value = JSON.parse(outputData.value);
    ElMessage({
        message: '启动成功!',
        type: 'success',
    })
}

async function output() {
    let status = await window.my.output()   ;
    if (status) {
        ElMessage({
            message: '保存成功!',
            type: 'success',
        })
    } else {
        ElMessage({
            message: '取消保存',
            type: 'success',
        })
    }
}
</script>

<style scoped>

</style>