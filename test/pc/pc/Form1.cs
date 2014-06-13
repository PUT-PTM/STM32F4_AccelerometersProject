using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using ZedGraph;

namespace pc
{
    public partial class Form1 : Form
    {
        // Starting time in milliseconds
        int tickStart = 0;
        LineItem curve_x;
        LineItem curve_y;
        LineItem curve_z;
        private SerialPort port = new SerialPort("COM3",
          9600, Parity.None, 8, StopBits.One);

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            GraphPane myPane = zedGraphControl1.GraphPane;
            myPane.Title.Text = "Offsets";

            port.DataReceived += new
              SerialDataReceivedEventHandler(port_DataReceived);

            // Begin communications
            port.Open();

            // Save 1200 points.  At 50 ms sample rate, this is one minute
            // The RollingPointPairList is an efficient storage class that always
            // keeps a rolling set of point data without needing to shift any data values
            RollingPointPairList list_x = new RollingPointPairList(1200);
            RollingPointPairList list_y = new RollingPointPairList(1200);
            RollingPointPairList list_z = new RollingPointPairList(1200);

            // Initially, a curve is added with no data points (list is empty)
            // Color is blue, and there will be no symbols
            curve_x = myPane.AddCurve("X", list_x, Color.Blue, SymbolType.None);
            curve_y = myPane.AddCurve("Y", list_y, Color.Green, SymbolType.None);
            curve_z = myPane.AddCurve("Z", list_z, Color.Pink, SymbolType.None);

            // Sample at 50ms intervals
            timer1.Tick += new EventHandler(timer1_Tick);
            timer1.Interval = 50;
            timer1.Enabled = true;
            timer1.Start();

            // Just manually control the X axis range so it scrolls continuously
            // instead of discrete step-sized jumps
            myPane.XAxis.Scale.Min = 0;
            myPane.XAxis.Scale.Max = 30;
            myPane.XAxis.Scale.MinorStep = 1;
            myPane.XAxis.Scale.MajorStep = 5;

            // Scale the axes
            zedGraphControl1.AxisChange();

            // Save the beginning time for reference
            tickStart = Environment.TickCount;
        }

        private void port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            // Show all the incoming data in the port's buffer
            byte[] bytes = Encoding.UTF8.GetBytes(port.ReadExisting());
            if (bytes.Length < 6)
            {
                return;
            }
            UInt16 x = BitConverter.ToUInt16(bytes, 0);
            UInt16 y = BitConverter.ToUInt16(bytes, 2);
            UInt16 z = BitConverter.ToUInt16(bytes, 4);

            // Get the PointPairList
            IPointListEdit list_x = curve_x.Points as IPointListEdit;
            IPointListEdit list_y = curve_y.Points as IPointListEdit;
            IPointListEdit list_z = curve_z.Points as IPointListEdit;
            // If this is null, it means the reference at curve.Points does not
            // support IPointListEdit, so we won't be able to modify it

            // Time is measured in seconds
            double time = (Environment.TickCount - tickStart) / 1000.0;

            // 3 seconds per cycle
            list_x.Add(time, x);
            list_y.Add(time, y);
            list_z.Add(time, z);

            // Keep the X scale at a rolling 30 second interval, with one
            // major step between the max X value and the end of the axis
            Scale xScale = zedGraphControl1.GraphPane.XAxis.Scale;
            if (time > xScale.Max - xScale.MajorStep)
            {
                xScale.Max = time + xScale.MajorStep;
                xScale.Min = xScale.Max - 30.0;
            }

            // Make sure the Y axis is rescaled to accommodate actual data
            zedGraphControl1.AxisChange();
            // Force a redraw
            zedGraphControl1.Invalidate();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            // sends 'u' char
            port.Write(new byte[] { 0x75 }, 0, 1);
            
        }

        private void Form1_Resize(object sender, EventArgs e)
        {
            SetSize();
        }

        // Set the size and location of the ZedGraphControl
        private void SetSize()
        {
            // Control is always 10 pixels inset from the client rectangle of the form
            Rectangle formRect = this.ClientRectangle;
            formRect.Inflate(-10, -10);

            if (zedGraphControl1.Size != formRect.Size)
            {
                zedGraphControl1.Location = formRect.Location;
                zedGraphControl1.Size = formRect.Size;
            }
        }

        private void zedGraphControl1_Load(object sender, EventArgs e)
        {

        }
    }
}
