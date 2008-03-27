namespace StochasticModeling
{
    partial class MasterPaneForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.MasterGraph = new ZedGraph.ZedGraphControl();
            this.SuspendLayout();
            // 
            // MasterGraph
            // 
            this.MasterGraph.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.MasterGraph.AutoScroll = true;
            this.MasterGraph.AutoSize = true;
            this.MasterGraph.EditButtons = System.Windows.Forms.MouseButtons.Left;
            this.MasterGraph.Location = new System.Drawing.Point(-16, 0);
            this.MasterGraph.Name = "MasterGraph";
            this.MasterGraph.PanModifierKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Shift | System.Windows.Forms.Keys.None)));
            this.MasterGraph.ScrollGrace = 0;
            this.MasterGraph.ScrollMaxX = 0;
            this.MasterGraph.ScrollMaxY = 0;
            this.MasterGraph.ScrollMaxY2 = 0;
            this.MasterGraph.ScrollMinX = 0;
            this.MasterGraph.ScrollMinY = 0;
            this.MasterGraph.ScrollMinY2 = 0;
            this.MasterGraph.Size = new System.Drawing.Size(951, 1082);
            this.MasterGraph.TabIndex = 2;
            // 
            // MasterPaneForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.AutoScroll = true;
            this.ClientSize = new System.Drawing.Size(950, 592);
            this.Controls.Add(this.MasterGraph);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "MasterPaneForm";
            this.Text = "Graph Collection";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.OnFormClosing);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private ZedGraph.ZedGraphControl MasterGraph;
    }
}