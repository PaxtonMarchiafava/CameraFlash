
## Electronics

#### Quiescent Current
| Item | Current draw in shutdown | Condition |
|:--:|:--:|:--:|
| ATtiny202 | 5u | Standby |
| LM2759 LED Driver | 9.7u | Shutdown |
| BQ25170 Battery Charger | 0.35u | unplugged |
| TOTAL | 15.05uA |  |

##### Battery life
$$
\begin{align}
\frac{Battery Capacity}{Current Consumption} =& Hours \\
\frac{Battery Capacity}{0.00001505 A} =& Hours \\
=& Hours
\end{align}
$$