# Anomaly Detection in Digital Transactions Using Social Network Data (Insight Data Engineering Challenge)

## Project Objective
Project URL: https://github.com/InsightDataScience/anomaly_detection

The challenge is to build a platform to analyze purchases within a social network of users, and detect any behavior that is far from the average within that social network. That is, a purchase amount is anomalous if it is more than 3 standard deviations from the mean of the last `T` purchases in the user's `D`th degree social network.

We are provided with the input files, `batch_log.json`, which contains past user network and purchase history data, and `stream_log.json`, which contains current data. Our goal is to flag & store anomalous purchases in `flagged_purchases.json`. 

## Updating Network State
Two main data structures are used,

`network`: contains information about friendships, i.e., hashtable where key is person's user id and value is set of the person's friends

`transactions`: contains information about previous purchases, i.e., hashtable where key is person's user id and value is multimap containing all previous transactions of the user

If an entry is `befriend` or `unfriend`, we update the corresponding element in `network`. Further, if an entry is `purchase`, we add it to `transactions`.

## Anomaly Detector
Given a new purchase entry, we find the `T` most recent purchases made by the up to `D`'th degree friends of the user. To do so, we perform a BFS up to level `D` and we use a heap data structure of size `T` to store the most recent purchases. Finally, we iterate through the heap, compute mean and standard deviation, and decide if the input entry is anomalous.


## Output 
The code generates `flagged_purchases.json` in the `log_output` directory, which contains all the anomalous purchases. 

During the test stages, the average runtime of the code is 3.48 seconds on a 3.4GHz processor with 8GB RAM.
