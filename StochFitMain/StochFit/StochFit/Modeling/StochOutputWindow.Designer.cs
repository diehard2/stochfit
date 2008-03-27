namespace StochasticModeling.Modeling
{
    partial class StochOutputWindow
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
            this.RhoGraph = new ZedGraph.ZedGraphControl();
            this.ReflGraph = new ZedGraph.ZedGraphControl();
            this.ParametersTB = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.SelModelBT = new System.Windows.Forms.Button();
            this.CancelCB = new System.Windows.Forms.Button();
            this.ModelLB = new System.Windows.Forms.ListBox();
            this.label2 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // RhoGraph
            // 
            this.RhoGraph.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.RhoGraph.AutoSize = true;
            this.RhoGraph.EditButtons = System.Windows.Forms.MouseButtons.Left;
            this.RhoGraph.Location = new System.Drawing.Point(472, 12);
            this.RhoGraph.Name = "RhoGraph";
            this.RhoGraph.PanModifierKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Shift | System.Windows.Forms.Keys.None)));
            this.RhoGraph.ScrollGrace = 0;
            this.RhoGraph.ScrollMaxX = 0;
            this.RhoGraph.ScrollMaxY = 0;
            this.RhoGraph.ScrollMaxY2 = 0;
            this.RhoGraph.ScrollMinX = 0;
            this.RhoGraph.ScrollMinY = 0;
            this.RhoGraph.ScrollMinY2 = 0;
            this.RhoGraph.Size = new System.Drawing.Size(272, 207);
            this.RhoGraph.TabIndex = 3;
            // 
            // ReflGraph
            // 
            this.ReflGraph.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.ReflGraph.AutoSize = true;
            this.ReflGraph.EditButtons = System.Windows.Forms.MouseButtons.Left;
            this.ReflGraph.Location = new System.Drawing.Point(472, 225);
            this.ReflGraph.Name = "ReflGraph";
            this.ReflGraph.PanModifierKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Shift | System.Windows.Forms.Keys.None)));
            this.ReflGraph.ScrollGrace = 0;
            this.ReflGraph.ScrollMaxX = 0;
            this.ReflGraph.ScrollMaxY = 0;
            this.ReflGraph.ScrollMaxY2 = 0;
            this.ReflGraph.ScrollMinX = 0;
            this.ReflGraph.ScrollMinY = 0;
            this.ReflGraph.ScrollMinY2 = 0;
            this.ReflGraph.Size = new System.Drawing.Size(272, 207);
            this.ReflGraph.TabIndex = 4;
            // 
            // ParametersTB
            // 
            this.ParametersTB.BackColor = System.Drawing.SystemColors.Window;
            this.ParametersTB.ForeColor = System.Drawing.Color.Black;
            this.ParametersTB.Location = new System.Drawing.Point(223, 34);
            this.ParametersTB.Multiline = true;
            this.ParametersTB.Name = "ParametersTB";
            this.ParametersTB.ReadOnly = true;
            this.ParametersTB.Size = new System.Drawing.Size(228, 292);
            this.ParametersTB.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(220, 12);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(73, 13);
            this.label1.TabIndex = 6;
            this.label1.Text = "Fit parameters";
            // 
            // SelModelBT
            // 
            this.SelModelBT.Location = new System.Drawing.Point(193, 345);
            this.SelModelBT.Name = "SelModelBT";
            this.SelModelBT.Size = new System.Drawing.Size(124, 37);
            this.SelModelBT.TabIndex = 2;
            this.SelModelBT.Text = "Select Current Model";
            this.SelModelBT.UseVisualStyleBackColor = true;
            this.SelModelBT.Click += new System.EventHandler(this.SelModelBT_Click);
            // 
            // CancelCB
            // 
            this.CancelCB.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.CancelCB.Location = new System.Drawing.Point(193, 394);
            this.CancelCB.Name = "CancelCB";
            this.CancelCB.Size = new System.Drawing.Size(124, 37);
            this.CancelCB.TabIndex = 3;
            this.CancelCB.Text = "Cancel";
            this.CancelCB.UseVisualStyleBackColor = true;
            // 
            // ModelLB
            // 
            this.ModelLB.FormattingEnabled = true;
            this.ModelLB.Location = new System.Drawing.Point(6, 36);
            this.ModelLB.Name = "ModelLB";
            this.ModelLB.Size = new System.Drawing.Size(194, 290);
            this.ModelLB.TabIndex = 0;
            this.ModelLB.SelectedIndexChanged += new System.EventHandler(this.OnModelIndexChange);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(3, 12);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(41, 13);
            this.label2.TabIndex = 10;
            this.label2.Text = "Models";
            // 
            // StochOutputWindow
            // 
            this.AcceptButton = this.SelModelBT;
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.CancelButton = this.CancelCB;
            this.ClientSize = new System.Drawing.Size(756, 443);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.ModelLB);
            this.Controls.Add(this.CancelCB);
            this.Controls.Add(this.SelModelBT);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.ParametersTB);
            this.Controls.Add(this.ReflGraph);
            this.Controls.Add(this.RhoGraph);
            this.MinimizeBox = false;
            this.Name = "StochOutputWindow";
            this.Text = "StochOutputWindow";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private ZedGraph.ZedGraphControl RhoGraph;
        private ZedGraph.ZedGraphControl ReflGraph;
        private System.Windows.Forms.TextBox ParametersTB;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button SelModelBT;
        private System.Windows.Forms.Button CancelCB;
        private System.Windows.Forms.ListBox ModelLB;
        private System.Windows.Forms.Label label2;
    }
}