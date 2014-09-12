using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using hundsun.t2sdk;

namespace extT2sdkTest
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private CT2Configinterface config = null;
        private CT2Connection conn = null;
        private callbacktest callback = null;

        private delegate void UpdateStatusDelegate(string status);

        private void UpdateStatus(string status)
        {
            this.textBox1.Text = string.Format("{0}\r\n{1}", status, textBox1.Text);
        }

        public void DisplayText(string szText)
        {
            this.BeginInvoke(new UpdateStatusDelegate(UpdateStatus), new object[] { szText });
        }

        private bool InitT2()
        {
            if (conn == null)
            {
                config = new CT2Configinterface();
                config.Load("t2sdk.ini");
                conn = new CT2Connection(config);
                callback = new callbacktest(this);
                conn.Create(callback);
                int iret = conn.Connect(4000);
                textBox1.Text = "连接" + conn.GetErrorMsg(iret) + "\r\n" + textBox1.Text;
                if (iret != 0)
                {
                    return false;
                }
            }

            return true;
        }

        private void CloseT2()
        {
            config.Dispose();
            conn.Dispose();
            config = null;
            conn = null;
            textBox1.Text = "";
        }

        private void button1_Click(object sender, EventArgs e)
        {
            //初始化t2连接
            if (!InitT2())
            {
                return;
            }

            //打包请求报文
            CT2Packer packer = new CT2Packer(2);
            sbyte strType = Convert.ToSByte('S');
            sbyte intType = Convert.ToSByte('I');
            packer.BeginPack();
            //插件编号
            packer.AddField("plugin_id", strType, 255, 4);
            //管理功能号
            packer.AddField("function_id", intType, 255, 4);
            packer.AddStr("com.hundsun.fbase.f2core");
            packer.AddInt(100);
            packer.EndPack();
            //功能号，业务包
            int iRet = conn.SendBiz(8, packer, 0, 0, 1);
            if (iRet < 0)
            {
                DisplayText(conn.GetErrorMsg(iRet));
            }
            else
            {
                string error = null;
                CT2UnPacker unpacker = null;
                iRet = conn.RecvBiz(iRet, out error, out unpacker, 4000, 0);
                if (iRet == 0 || iRet == 1)
                {
                    while (unpacker.IsEOF() != 1)
                    {
                        for (int i = 0; i < unpacker.GetColCount(); i++)
                        {
                            textBox1.Text += unpacker.GetStrByIndex(i) + " ";
                        }
                        textBox1.Text += "\r\n";
                        unpacker.Next();
                    }
                }
                else if (iRet < 0)
                {
                    DisplayText(conn.GetErrorMsg(iRet));
                }
                else if (iRet == 2)
                {
                    DisplayText("解包失败");
                }
            }

            packer.Dispose();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            //打包请求报文
            CT2Packer packer = new CT2Packer(2);
            sbyte strType = Convert.ToSByte('S');
            sbyte intType = Convert.ToSByte('I');
            packer.BeginPack();
            //插件编号
            packer.AddField("plugin_id", strType, 255, 4);
            //管理功能号
            packer.AddField("function_id", intType, 255, 4);
            packer.AddStr("com.hundsun.fbase.f2core");
            packer.AddInt(100);
            packer.EndPack();

            int iRet = conn.SendBiz(8, packer, 1, 0, 1);

            packer.Dispose();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            //初始化t2连接
            if (!InitT2())
            {
                return;
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            CloseT2();
        }

        private void button5_Click(object sender, EventArgs e)
        {
            //textBox1.Text = conn.test();
        }

    }

    public unsafe class callbacktest : CT2CallbackInterface
    {
        public callbacktest(Form1 form1) { m_lpOwner = form1; }

        private Form1 m_lpOwner;

        public override void OnConnect(CT2Connection lpConnection)
        {
            System.Console.WriteLine("OnConnect");
        }
        public override void OnSafeConnect(CT2Connection lpConnection)
        {
            System.Console.WriteLine("OnSafeConnect");
        }
        public override void OnRegister(CT2Connection lpConnection)
        {
            System.Console.WriteLine("OnRegister");
        }
        public override void OnClose(CT2Connection lpConnection)
        {
            System.Console.WriteLine("OnClose");
        }

        public override void OnReceivedBiz(CT2Connection lpConnection, int hSend, String lppStr, CT2UnPacker lppUnPacker, int nResult)
        {
            if (nResult == 0 || nResult == 1)
            {
                while (lppUnPacker.IsEOF() != 1)
                {
                    for (int i = 0; i < lppUnPacker.GetColCount(); i++)
                    {
                        m_lpOwner.DisplayText(lppUnPacker.GetStrByIndex(i));
                    }
                    lppUnPacker.Next();
                }
            }
            else if (nResult < 0)
            {
                m_lpOwner.DisplayText(lpConnection.GetErrorMsg(nResult));
            }
            else if (nResult == 2)
            {
                m_lpOwner.DisplayText("解包失败");
            }
        }

		public override void OnReceivedBizEx(CT2Connection lpConnection, int hSend, CT2RespondData  lpRetData, String lppStr, CT2UnPacker lppUnPacker, int nResult)
        {
        }

        public override void OnSent(CT2Connection lpConnection, int hSend, void* lpData, int nLength, int nQueuingData)
        {

        }
    };
}