#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class Solution
{
public:
    int solve(vector<int> &field, int n)
    {
        int m = field.size();
        //从小到大排序
        sort(field.begin(), field.end());
        if(m>n) return -1;
        int maxk = *max_element(field.begin(), field.end());
        int l=0,r=maxk;
        auto check = [&](int k)->bool{
            //记录当前用了多长时间
            int tot = 0;
            auto it = lower_bound(field.begin(), field.end(),k);
            //k大于最大的面积
            if(it == field.end())
            {
                return m <= n;
            }
            else
            {
                //前面的用时
                tot = it-field.begin();
                for(;it!=field.end();it++)
                {
                    //施肥完这块地需要时间向上取整
                    int tmp = *it/k + (*it%k==0?0:1);
                    tot +=tmp;
                    if(tot > n) return false;
                }
                return tot > n ? false:true;
            }
        };
        int ret = -1;
        while(l<r)
        {
            int mid = (r-l)/2 + l;
            bool ans = check(mid);
            if(ans)
            {
                ret = mid;
                r = mid-1;
            }
            else
            {
                l=mid+1;
            }
        }
        return ret;
    }
private:

};

int main()
{
    int m=0,n=0;
    Solution s;
    vector<int> vec;
    while(cin>>m && cin>>n)
    {
        int x;
        vec.clear();
        for(int i=0;i<m;i++)
        {
            cin>>x;
            vec.push_back(x);
        }
        if(m > n) printf("-1\n");
        else
        {
            int ret = s.solve(vec,n);
            printf("%d",ret);
        }
    }
    return 0;
}