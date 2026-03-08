import streamlit as st
import pandas as pd
import requests
import plotly.express as px
from datetime import datetime, timedelta
from streamlit_autorefresh import st_autorefresh

# --------------------------------------------------
# CONFIGURATION
# --------------------------------------------------

FIREBASE_URL = "https://smartdooriot-fad5e-default-rtdb.firebaseio.com/access_logs.json"

USERNAME = "admin"
PASSWORD = "smartdoor"

st.set_page_config(
    page_title="SmartDoor Security Dashboard",
    layout="wide"
)

# --------------------------------------------------
# LOGIN SYSTEM
# --------------------------------------------------

def login():

    st.title("🔐 SmartDoor Secure Login")

    user = st.text_input("Username")
    pwd = st.text_input("Password", type="password")

    if st.button("Login"):

        if user == USERNAME and pwd == PASSWORD:
            st.session_state["authenticated"] = True
            st.rerun()

        else:
            st.error("Invalid credentials")

if "authenticated" not in st.session_state:
    st.session_state["authenticated"] = False

if not st.session_state["authenticated"]:
    login()
    st.stop()

# --------------------------------------------------
# AUTO REFRESH
# --------------------------------------------------

st_autorefresh(interval=5000, key="refresh")

# --------------------------------------------------
# HEADER
# --------------------------------------------------

st.title("SmartDoor IoT Security Monitoring System")

# --------------------------------------------------
# FETCH FIREBASE DATA
# --------------------------------------------------

def fetch_data():

    response = requests.get(FIREBASE_URL)
    data = response.json()

    rows = []

    if data:

        for key in data:

            entry = data[key]

            rows.append({
                "Device": entry["device"],
                "Status": entry["status"],
                "Timestamp": entry["timestamp"]
            })

    df = pd.DataFrame(rows)

    return df


df = fetch_data()

if df.empty:
    st.warning("No access events recorded yet.")
    st.stop()

df["Timestamp"] = pd.to_datetime(df["Timestamp"])
df = df.sort_values("Timestamp", ascending=False)

# --------------------------------------------------
# TIME RANGE SELECTOR
# --------------------------------------------------

st.sidebar.header("Time Range Filter")

time_range = st.sidebar.selectbox(
    "Select Range",
    ["Today", "Last 24 Hours", "Last Week"]
)

now = datetime.now()

if time_range == "Today":
    df_filtered = df[df["Timestamp"].dt.date == now.date()]

elif time_range == "Last 24 Hours":
    df_filtered = df[df["Timestamp"] > now - timedelta(hours=24)]

else:
    df_filtered = df[df["Timestamp"] > now - timedelta(days=7)]

# --------------------------------------------------
# ACCESS METRICS
# --------------------------------------------------

granted = len(df_filtered[df_filtered["Status"] == "granted"])
denied = len(df_filtered[df_filtered["Status"] == "denied"])

col1, col2 = st.columns(2)

col1.metric("✅ Granted Access", granted)
col2.metric("❌ Denied Attempts", denied)

# --------------------------------------------------
# SECURITY ALERT
# --------------------------------------------------

recent_denied = df[
    (df["Status"] == "denied") &
    (df["Timestamp"] > now - timedelta(minutes=2))
]

if len(recent_denied) >= 3:

    st.error("🚨 SECURITY ALERT: Multiple denied access attempts detected!")

# --------------------------------------------------
# ACCESS SUMMARY CHART
# --------------------------------------------------

st.subheader("Access Summary")

summary_data = pd.DataFrame({
    "Access Type": ["Granted", "Denied"],
    "Count": [granted, denied]
})

fig = px.bar(
    summary_data,
    x="Access Type",
    y="Count",
    color="Access Type",
    text="Count"
)

st.plotly_chart(fig, use_container_width=True)

# --------------------------------------------------
# ACCESS TIMELINE
# --------------------------------------------------

st.subheader("Access Timeline")

timeline = df_filtered.copy()

timeline["Time"] = timeline["Timestamp"].dt.strftime("%H:%M")

timeline_chart = px.scatter(
    timeline,
    x="Timestamp",
    y="Status",
    color="Status",
    title="Access Events Over Time"
)

st.plotly_chart(timeline_chart, use_container_width=True)

# --------------------------------------------------
# DAILY ACCESS STATISTICS
# --------------------------------------------------

st.subheader("Daily Access Statistics")

daily_stats = df.copy()

daily_stats["Date"] = daily_stats["Timestamp"].dt.date

daily_summary = daily_stats.groupby(["Date","Status"]).size().reset_index(name="Count")

daily_chart = px.bar(
    daily_summary,
    x="Date",
    y="Count",
    color="Status",
    barmode="group"
)

st.plotly_chart(daily_chart, use_container_width=True)

# --------------------------------------------------
# ACCESS LOG TABLE
# --------------------------------------------------

st.subheader("Access Logs")

st.dataframe(
    df,
    use_container_width=True,
    height=450
)

# --------------------------------------------------
# FOOTER
# --------------------------------------------------

st.markdown("---")
st.caption("SmartDoor IoT Security System")