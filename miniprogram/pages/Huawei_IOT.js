// pages/Huawei_IOT.js
const CONFIG = require('../../config.js')
Page({

    /**
     * 页面的初始数据
     */
    data: {
        result:'等待获取token',
        c1i1:0,
        c1i2:0,
        c1i3:0,
    },
    /**
     * 获取token按钮按下：
     */
    touchBtn_gettoken:function()
    {
        console.log("获取token按钮按下动作");
        this.setData({result:'获取token按钮按下动作'});
        this.gettoken();
    },
    /**
     * 获取设备影子按钮按下：
     */
    touchBtn_getshadow:function()
    {
        console.log("获取设备影子按钮按下动作");
        this.setData({result:'获取设备影子按钮按下动作'});
        this.getshadow();
    },
     /**
     * 设备命令下发按钮按下：
     */
    touchBtn_setCommand:function()
    {
        console.log("设备命令下发按钮按下");
        this.setData({result:'设备命令下发按钮按下，正在下发...'});
        this.setCommand();
    },  
    /**
     * 获取token
     */
    gettoken:function(){
        console.log("开始获取。。。");//打印完整消息
        var that=this;  //这个很重要，在下面的回调函数中由于异步问题不能有效修改变量，需要用that获取
        
        wx.request({
            url: 'https://iam.cn-north-4.myhuaweicloud.com/v3/auth/tokens',
            data: JSON.stringify({
                auth: {
                    identity: {
                        methods: ['password'],
                        password: {
                            user: {
                                name: CONFIG.IAM_USERNAME,
                                password: CONFIG.IAM_PASSWORD,
                                domain: { name: CONFIG.IAM_DOMAIN }
                            }
                        }
                    },
                    scope: { project: { name: CONFIG.IAM_PROJECT } }
                }
            }),
            method: 'POST', 
            // OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE, CONNECT
            header: {'content-type': 'application/json' }, // 请求的 header 
            success: function(res){// success
              // success
                console.log("获取token成功");//打印完整消息
                console.log(res);//打印完整消息
                
                var token='';
                token=JSON.stringify(res.header['X-Subject-Token']);
                //解析消息头的token
                token=token.replaceAll("\"", "");
                console.log("获取token=\n"+token);//打印token
                wx.setStorageSync('token',token);
                that .setData({ Text: 'Token数据：'+token});
                //把token写到缓存中,以便可以随时随地调用
            },
            fail:function(){
                // fail
                console.log("获取token失败");//打印完整消息
            },
            complete: function() {
                // complete
                console.log("获取token完成");//打印完整消息
            } 
        });
    },
    /**
     * 获取设备影子
     */
    getshadow:function(){
        console.log("开始获取影子");//打印完整消息
        var that=this; 
    //这个很重要，在下面的回调函数中由于异步问题不能有效修改变量，需要用that获取
        var token=wx.getStorageSync('token');//读缓存中保存的token
        wx.request({
            url: `${CONFIG.IOTDA_ENDPOINT}/v5/iot/${CONFIG.PROJECT_ID}/devices/${CONFIG.DEVICE_ID}/shadow`,
            data:'',
            method: 'GET', // OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE, CONNECT
            header: {'content-type': 'application/json','X-Auth-Token':token }, //请求的header 
            success: function(res){// success
            // success
                console.log(res);//打印完整消息
                
                var shadow=JSON.stringify(res.data.shadow[0].reported.properties);
                console.log('设备影子数据：'+shadow);
                that .setData({ Text: '设备影子数据：'+shadow})
            },
            fail:function(){
                // fail
                console.log("获取影子失败");//打印完整消息
                console.log("请先获取token");//打印完整消息
            },
            complete: function() {
                // complete
                console.log("获取影子完成");//打印完整消息
            } 
        });
    },
    /**
     * 设备命令下发
     */
    setCommand:function(){
        console.log("开始下发命令。。。");//打印完整消息
        var that=this; 
    //这个很重要，在下面的回调函数中由于异步问题不能有效修改变量，需要用that获取
        var token=wx.getStorageSync('token');//读缓存中保存的token
        
        var strdata='{"paras":{"cmd1int1":'+this.data.c1i1+',"cmd1int2":'+this.data.c1i2+',"cmd1int3":'+this.data.c1i3+'},"service_id":"fuwu","command_name":"cmd1"}';
        this.setData({ Text:strdata});
        wx.request({
            url: `${CONFIG.IOTDA_ENDPOINT}/v5/iot/${CONFIG.PROJECT_ID}/devices/${CONFIG.DEVICE_ID}/commands`,
            //data:{"paras":{"cmd1int1":30,"cmd1int2":30,"cmd1int3":30},"service_id":"fuwu","command_name":"cmd1"},
            data:strdata,
            method: 'POST', // OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE, CONNECT
            header: {'content-type': 'application/json','X-Auth-Token':token }, //请求的header 
            
            success: function(res){// success
                // success
                console.log("下发命令成功");//打印完整消息
                console.log(res);//打印完整消息
                that.setData({result:'下发成功（响应状态码'+ res.statusCode+')'});
             },
              
             fail:function(res){
                // fail
                console.log(res);//打印完整消息
                that.setData({result:'下发失败：发生未知的通信错误（响应状态码'+ res.statusCode+')'});
            },
            
            complete: function(res) {
                // complete
                //console.log("命令下发完成");//打印完整消息
            } 
        });
    },
//清除按钮
kill:function()
{
  console.log("kill按钮");//打印完整消息
  this.setData({ Text: ' '})
},


//获取滑动后的值
slidervalue1:function(e) {
  console.log(e.detail.value);
  this.data.c1i1=e.detail.value;
  var str="cmd1int1的值为："+this.data.c1i1;
  this.setData({ Text:str});
},
slidervalue2:function(e) {
  console.log(e.detail.value);
  this.data.c1i2=e.detail.value;
  var str="cmd1int2的值为："+this.data.c1i2;
  this.setData({ Text:str});
},
slidervalue3:function(e) {
  console.log(e.detail.value);
  this.data.c1i3=e.detail.value;
  var str="cmd1int3的值为："+this.data.c1i3;
  this.setData({ Text:str});
},

//测试按钮
ceii:function()
{
  //this.setData({ Text: 'ceii'});
  //this.setData({ Text: e.detail.value});
  
  //this.setData({ Text: upLightValue});
},
      
    /**
     * 生命周期函数--监听页面加载
     */
    onLoad(options) {

    },

    /**
     * 生命周期函数--监听页面初次渲染完成
     */
    onReady() {

    },

    /**
     * 生命周期函数--监听页面显示
     */
    onShow() {

    },

    /**
     * 生命周期函数--监听页面隐藏
     */
    onHide() {

    },

    /**
     * 生命周期函数--监听页面卸载
     */
    onUnload() {

    },

    /**
     * 页面相关事件处理函数--监听用户下拉动作
     */
    onPullDownRefresh() {

    },

    /**
     * 页面上拉触底事件的处理函数
     */
    onReachBottom() {

    },

    /**
     * 用户点击右上角分享
     */
    onShareAppMessage() {

    }
})